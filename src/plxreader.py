import struct
import os

__all__ = ['PLXReader']


class PLXReader:
    # Properties -----------------------------------------------------------
    # private:
    _MAGIC = 0x58454c50
    # theses are calculated from Plexon.h
    _L_FILE_HEADER = 7504
    _L_CHN_HEADER = 1020
    _L_CHN_HEADER_SKIP = 2*5*64 + 4*5 + 4 + 2*5*2*4 + 4 + 128 + 4*11
    _L_EV_HEADER = 296
    _L_SLOW_HEADER = 296
    _L_BYTES_PER_ONE_WAVEFORM_SAMPLE = 2
    _L_PACKET = 16
    _I_STROBED = 257
    _I_PLX_SPIKES = 1
    _I_PLX_EXTEV = 4
    _I_PLX_SLOW = 5

    @property
    def valid(self):
        return self._valid
    @property
    def t_sample(self):
        return self._t_sample
    @property
    def chn_info(self):
        return self._chn_info.copy()
    @property
    def spike_id(self):
        return self._spike_id[:]

    # Methods -------------------------------------------------------------
    def __init__(self, plx_filename=None):
        self.set_files(plx_filename=plx_filename)
        self._plx_fp = None
        self._reset()


    def _reset(self):
        if self._plx_fp != None:
            self._plx_fp.close()

        self._plx_fp = None
        self._i_version = -1
        self._valid = False
        self._t_res = -1
        self._t_sample = -1
        self._chn_info = None
        self._s_comment = None
        self._p_databegin = -1
        self._spike_id = None


    def set_files(self, plx_filename=None):
        self.plx_filename = plx_filename


    def close(self):
        if self._plx_fp != None:
            self._plx_fp.close()
        self._plx_fp = None
        self._reset()


    def open(self):
        self._open_plx()

    # plx file header reader -----------------------------------------------
    def _open_plx(self):
        plx_filename = self.plx_filename
        if plx_filename is None: return False
        f = open(plx_filename, 'rb')

        # header and packet size
        i_magic, i_version, s_comment = struct.unpack('<Ii128s', f.read(136))
        if i_magic != PLXReader._MAGIC:
            print 'Error: bad magic bytes'
            return False
        # timestamp freq, # DSP ch, # event ch, # slow ch
        t_res, n_dspch, n_evch, n_slowch = struct.unpack('<4i', f.read(16))
        t_res = 1000000. / float(t_res)         # converting to us
        # # data pt in waveform, # data pt before cross threshold
        n_ptwav, n_ptprethr = struct.unpack('<2i', f.read(8))
        # year, month, day, hour, minute, second
        i_year, i_month, i_day, i_hour, i_min, i_sec = \
                struct.unpack('<6i', f.read(24))
        # fast read (reserved), waveform sampling freq, experiment duration in ticks
        i_fastread, t_sample, t_lasttime = struct.unpack('<2id', f.read(16))
        if t_sample == 0: t_sample = t_res
        else: t_sample = 1000000. / float(t_sample)   # converting to us
        # trodalness, data trodalness, bits/spike_sample, bits/slow_sample
        i_trodalness, i_datatrodalness, n_bitsspk, n_bitsslow = \
                struct.unpack('<4B', f.read(4))
        # max spike mV, max slow mV
        i_spkamplitude, i_slowamplitude = struct.unpack('<2H', f.read(4))
        # spike preamp gain
        i_preampgain = struct.unpack('<H', f.read(2))
        # padding
        x = f.read(46)
        # skip rest in the file header.
        f.seek(PLXReader._L_FILE_HEADER)

        # read all dsp channel --
        chn_info = {}
        spike_id = []
        for i in range(n_dspch):
            s_name = f.read(32)
            s_signame = f.read(32)
            # DSP channel #, WF reate, SIG chn #, Reference chn #
            elec_id, wf_rate, i_sig, i_ref = struct.unpack('<4i', f.read(16))
            # filter used?, threshold, shorting method, # units sorted
            i_gain, b_filer, i_thr, i_metd, n_units = struct.unpack('<5i', f.read(20))
            # ignore rest...
            f.seek(PLXReader._L_CHN_HEADER_SKIP, os.SEEK_CUR)

            # dig_nv: digitization factor in nV
            # TODO: cannot understand plxon's gain and digitization fully.
            # must fix `dig_nv` correctly
            chn_info[elec_id] = {'dig_nv': 3000./i_gain, 
                                 'n_bwav': PLXReader._L_BYTES_PER_ONE_WAVEFORM_SAMPLE} 
            spike_id.append(elec_id)

        # where's the actual data?
        data_begin = PLXReader._L_FILE_HEADER + n_dspch * PLXReader._L_CHN_HEADER + \
                     n_evch * PLXReader._L_EV_HEADER + \
                     n_slowch * PLXReader._L_SLOW_HEADER

        # done.
        self._plx_fp = f
        self._i_version = i_version
        self._valid = True
        self._t_res = t_res
        self._t_sample = t_sample
        self._chn_info = chn_info
        self._s_comment = s_comment
        self._p_databegin = data_begin
        self._spike_id = spike_id

        self.goto_first_data()

        return True

    # .plx file methods ====================================================================
    def read_once(self, pos=None, proc_wav=False, get_others=False):
        """read one particular block: must be on the right position!!!"""
        f = self._plx_fp
        t_res = self._t_res
        chn_info = self._chn_info

        if pos != None: f.seek(pos)
        while True:
            pos = f.tell()
            block_buf = f.read(PLXReader._L_PACKET)
            if block_buf == '': return None   # reached EOF

            # Type, upper ts, lower ts, channel #, unit, # waves, # samples/wave
            i_type, t_upper, t_lower, sig_id, i_unit, n_waves, n_samples = \
                    struct.unpack('<hHL4h', block_buf)
            l_waveform = []

            # read waveforms if there's any
            if n_waves > 0:
                n_shorts = n_waves * n_samples
                fmt = '<' + 'h' * n_shorts
                l_waveform = struct.unpack(fmt, f.read(n_shorts*2))

            t0 = (t_upper << 32) + t_lower
            sig_t = t0 * t_res   # converting to us
            # ... finished reading the block ------------------------
            all_data = {'id':sig_id, 'timestamp':sig_t, 'file_pos':pos}

            # if it is strobed timestamp information:
            if sig_id == PLXReader._I_STROBED:
                all_data['data'] = i_unit
                break

            # if it's spike:
            elif i_type == PLXReader._I_PLX_SPIKES:
                if proc_wav: all_data['waveform'] = l_waveform
                break

                ## elec_data = {'waveform': l_waveform}
                # DEBUG {
                # print '# Samples:', len(points)
                # pl.plot(points)
                # pl.show()
                # return -1 
                # DEBUG }
                # TODO: elec_data assignment

            elif get_others:
                break

        return all_data


    def goto_first_data(self):
        self._plx_fp.seek(self._p_databegin)   # to the beginning of data


    # quick-and-dirty plx file reader -------------------------------------
    def extract_all(self, proc_wav=False, only_timestamp=False):
        if not self._valid:
            return False
        self.goto_first_data()

        # read all data signal blocks ----------
        t_timestamp = []    # when Plexon got the time stamps?  (in us)
        d_timestamp = []    # what is the actual value of the stamps?
        i_timestamp = []    # index in "whole_signal"
        all_signal = []     # [(timestamp, data), ...] 

        while True:
            the_data = self.read_once(proc_wav=proc_wav)
            if the_data == None: break

            # if it is strobed timestamp information:
            if the_data['id'] == PLXReader._I_STROBED:
                sig_t, data = the_data['timestamp'], the_data['data']
                t_timestamp.append(sig_t)
                d_timestamp.append(data)
                all_signal.append(the_data)
                i_timestamp.append(len(all_signal) - 1)

            # if it's spike:
            elif not only_timestamp:
                all_signal.append(the_data)

        return t_timestamp, d_timestamp, i_timestamp, all_signal
