/*
	Neutrino-GUI  -   DBoxII-Project


	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef __streaminfo2__
#define __streaminfo2__

#include <gui/widget/menue.h>

#include <driver/framebuffer.h>
#include <driver/pig.h>
#include <gui/widget/progressbar.h>


class CStreamInfo2 : public CMenuTarget
{
	private:

		CFrameBuffer	*frameBuffer;
		int x;
		int y;
		int width;
		int height;
		int hheight,iheight,sheight; 	// head/info/small font height

		int  max_height;	// Frambuffer 0.. max
		int  max_width;

		int yypos;
		int  paint_mode;

		int  font_head;
		int  font_info;
		int  font_small;

		int   sigBox_x;
		int   sigBox_y;
		int   sigBox_w;
		int   sigBox_h;
		int   sigBox_pos;
		int   sig_text_y;
		int   sig_text_ber_x;
		int   sig_text_sig_x;
		int   sig_text_snr_x;
		int   sig_text_rate_x;
		unsigned int scaling;
		struct feSignal {
			unsigned long	ber, old_ber, max_ber, min_ber;
			unsigned long	sig, old_sig, max_sig, min_sig;
			unsigned long	snr, old_snr, max_snr, min_snr;
		} signal;

		struct bitrate {
			unsigned int short_average, max_short_average, min_short_average;
		} rate;

		int  doSignalStrengthLoop();

		int dvrfd, dmxfd;
		struct timeval tv, last_tv, first_tv;
		uint64_t bit_s;
		uint64_t abit_s;
		uint64_t b_total;

		int update_rate();
		int ts_setup();
		int ts_close();

		void paint(int mode);
		void paint_pig(int x, int y, int w, int h);
		void paint_techinfo(int x, int y);
		void paint_signal_fe_box(int x, int y, int w, int h);
		void paint_signal_fe(struct bitrate rate, struct feSignal s);
		int  y_signal_fe(unsigned long value, unsigned long max_range, int max_y);
		void SignalRenderStr (unsigned int value, int x, int y);
		CProgressBar *sigscale;
		CProgressBar *snrscale;
		int lastsig, lastsnr;
		void showSNR ();

	public:

		CStreamInfo2();
		~CStreamInfo2();
		int exec();

		void hide();
		int exec(CMenuTarget* parent, const std::string & actionKey);

};
class CStreamInfo2Handler : public CMenuTarget
{
        public:
                int exec( CMenuTarget* parent,  const std::string &actionKey);
};
#endif

