#include <alsa/asoundlib.h>
#include <iostream>
#include <cstring>
#include <sndfile.h>

// threshold 以上の大きさの音を前後 margin 秒を含めて録音し
// wav 形式で標準出力に吐き出す。最大 1 分。
//
// yagrec [-D dev] [-m margin_in_second] [-t threshold(0-32767)]

using namespace std;

#define MAX_REC_SECOND 60

int main(int argc, char *argv[])
{
  int i;
  int ptrCur, ptrStart, ptrEnd;
  int err;
  int marginMs = 100;
  int threshold = 500;
  int cnt1shot;      // 信号が持続するかのカウンタ
  int cntSound;     //  正なら有音時間, 負なら無音時間をカウント
  unsigned int rate = 16000;
  string alsaDevice = "hw:0,0";
  snd_pcm_t *hCapture;
  snd_pcm_hw_params_t *params;
  short *buf;
  enum ERecState {eWAIT, eREC, eFINISH} recState;

  if ((buf = new short[rate * MAX_REC_SECOND]) == NULL) {
    cerr << "Cannot allocate memory.\n";
    return -1;
  }

  // parse args
  for (i = 0; i < argc; i++) {
    if (strcmp("-D", argv[i]) == 0) {
      i++;
      if (i < argc) {
	alsaDevice = argv[i];
      }
    }
    if (strcmp("-m", argv[i]) == 0) {
      i++;
      if (i < argc) {
	marginMs = (int) (atof(argv[i]) * 1000);
      }
    }
    if (strcmp("-t", argv[i]) == 0) {
      i++;
      if (i < argc) {
	threshold = (int) atoi(argv[i]);
      }
    }
    if (strcmp("-h", argv[i]) == 0) {
      cerr << "usage: yagrec [-D dev] [-m margin_in_second] [-t threshold(0-32767)]\n";
      return 0;
    }
  }

  if ((err = snd_pcm_open(&hCapture, alsaDevice.c_str(), SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    cerr << "Cannot open " << alsaDevice << ".\n" << snd_strerror(err) << endl;
    return -1;
  }

  if ((err = snd_pcm_hw_params_malloc(&params)) << 0) {
    cerr << snd_strerror(err) << endl;
    return -1;
  }

  if ((err = snd_pcm_hw_params_any(hCapture, params)) < 0) {
    cerr << snd_strerror(err) << endl;
    return -1;
  }

  if ((err = snd_pcm_hw_params_set_access(hCapture, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    cerr << snd_strerror(err) << endl;
    return -1;
  }

  if ((err = snd_pcm_hw_params_set_format(hCapture, params, SND_PCM_FORMAT_S16_LE)) < 0) {
    cerr << snd_strerror(err) << endl;
    return -1;
  }

  if ((err = snd_pcm_hw_params_set_rate_near(hCapture, params, &rate, 0)) < 0) {
    cerr << snd_strerror (err) << endl;
    return -1;
  }
	
  if ((err = snd_pcm_hw_params_set_channels(hCapture, params, 1)) < 0) {
    cerr << snd_strerror (err) << endl;
    return -1;
  }
	
  if ((err = snd_pcm_hw_params(hCapture, params)) < 0) {
    cerr << snd_strerror (err) << endl;
    return -1;
  }
	
  snd_pcm_hw_params_free(params);
	
  if ((err = snd_pcm_prepare(hCapture)) < 0) {
    cerr << snd_strerror (err) << endl;
    return -1;
  }

  cerr << "ready\n";
  // main loop
  for (ptrCur = 0, recState = eWAIT, cnt1shot = 0;
       recState != eFINISH;
       ptrCur = (ptrCur + 1) % (rate * MAX_REC_SECOND)) {
    if (cnt1shot > 0) cnt1shot--;

    if ((err = snd_pcm_readi(hCapture, &buf[ptrCur], 1)) != 1) continue;
    if (buf[ptrCur] > threshold) {
      cnt1shot = 100;
    }
    if (cnt1shot > 0) {
      if (cntSound < 0) cntSound = 0;
      if (++cntSound > rate * 0.1) {   // 有音 0.1 秒持続
	if (recState == eWAIT) {
	  recState = eREC;
	  cerr << "start\n";
	  ptrStart = (ptrCur + rate * MAX_REC_SECOND - (int)rate * marginMs / 1000) % (rate * MAX_REC_SECOND);
	}
      }
    } else {
      if (cntSound > 0) cntSound = 0;
      if (--cntSound < -0.5 * rate) {   // 無音 0.5 秒持続
	if (recState == eREC) {
	  cerr << "finish\n";
	  recState = eFINISH;
	  ptrEnd = ptrCur;
	}
      }
    }
  }

  // output
  SF_INFO sfinfo;
  SNDFILE *sf;
  sfinfo.samplerate = rate;
  sfinfo.channels = 1;
  sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
  sf = sf_open_fd(1, SFM_WRITE, &sfinfo, false);
  for (ptrCur = ptrStart;
       ptrCur != ptrEnd;
       ptrCur = (ptrCur + 1) % (rate * MAX_REC_SECOND)) {
    sf_writef_short(sf, &buf[ptrCur], 1);
  }
  sf_close(sf);
  return 0;
}

