/*
 * lua simple hint window
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

#ifndef _LUAHINT_H_
#define _LUAHINT_H_

#include <gui/widget/hintbox.h>

class CLuaHint
{
	public:
		CHint *h;
		CLuaHint() { h = NULL; }
		~CLuaHint() { delete h; }
};

class CLuaInstHint
{
	public:
		CLuaInstHint() {};
		~CLuaInstHint() {};
		static CLuaInstHint *getInstance();
		static void HintRegister(lua_State *L);

	private:
		static CLuaHint *HintCheck(lua_State *L, int n);

		static int HintNew(lua_State *L);
		static int HintPaint(lua_State *L);
		static int HintHide(lua_State *L);
		static int HintDelete(lua_State *L);
};

#endif //_LUAHINT_H_
