/*
 * lua video functions
 *
 * (C) 2014 [CST ]Focus
 * (C) 2014-2015 M. Liebmann (micha-bbg)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _LUAVIDEO_H
#define _LUAVIDEO_H

/*
class CLuaVideo
{
	public:
		CLuaVideo() {};
		~CLuaVideo() {};
};
*/

class CLuaInstVideo
{
	static CLuaData *CheckData(lua_State *L, int narg);
	public:
		CLuaInstVideo() {};
		~CLuaInstVideo() {};
		static CLuaInstVideo* getInstance();
//		static void LuaVideoRegister(lua_State *L);

		static int setBlank_old(lua_State *L);
		static int ShowPicture_old(lua_State *L);
		static int StopPicture_old(lua_State *L);
		static int PlayFile_old(lua_State *L);
		static int zapitStopPlayBack_old(lua_State *L);
		static int channelRezap_old(lua_State *L);

//	private:
};

#endif //_LUAVIDEO_H
