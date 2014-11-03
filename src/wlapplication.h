/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_WLAPPLICATION_H
#define WL_WLAPPLICATION_H

//Workaround for bug http://sourceforge.net/p/mingw/bugs/2152/
#ifdef __MINGW32__
#ifndef _WIN64
#define _USE_32BIT_TIME_T 1
#endif
#endif

#include <cassert>
#include <cstring>
#include <map>
#include <stdexcept>
#include <string>

#include <SDL_events.h>
#include <SDL_keyboard.h>

#include "base/point.h"


namespace Widelands {class Game;}

namespace UI
{

// Contains font information for a locale
struct FontSet {
public:
	// Wiriting diection of a script
	enum class Direction: uint8_t {
		kLeftToRight,
		kRightToLeft
	};

	FontSet(const std::string& serif_ = "DejaVuSerif.ttf",
			  const std::string& serif_bold_ = "DejaVuSerifBold.ttf",
			  const std::string& serif_italic_ = "DejaVuSerifItalic.ttf",
			  const std::string& serif_bold_italic_ = "DejaVuSerifBoldItalic.ttf",
			  const std::string& sans_ = "DejaVuSans.ttf",
			  const std::string& sans_bold_ = "DejaVuSansBold.ttf",
			  const std::string& sans_italic_ = "DejaVuSansItalic.ttf",
			  const std::string& sans_bold_italic_ = "DejaVuSansBoldItalic.ttf",
			  const std::string& condensed_ = "DejaVuSansCondensed.ttf",
			  const std::string& condensed_bold_ = "DejaVuSansCondensedBold.ttf",
			  const std::string& condensed_italic_ = "DejaVuSansCondensedItalic.ttf",
			  const std::string& condensed_bold_italic_ = "DejaVuSansCondensedBoldItalic.ttf",
			  const std::string& direction_ = "ltr") :
		m_serif(serif_),
		m_serif_bold(serif_bold_),
		m_serif_italic(serif_italic_),
		m_serif_bold_italic(serif_bold_italic_),
		m_sans(sans_),
		m_sans_bold(sans_bold_),
		m_sans_italic(sans_italic_),
		m_sans_bold_italic(sans_bold_italic_),
		m_condensed(condensed_),
		m_condensed_bold(condensed_bold_),
		m_condensed_italic(condensed_italic_),
		m_condensed_bold_italic(condensed_bold_italic_) {

		assert(!m_serif.empty());
		assert(!m_serif_bold.empty());
		assert(!m_serif_italic.empty());
		assert(!m_serif_bold_italic.empty());
		assert(!m_sans.empty());
		assert(!m_sans_bold.empty());
		assert(!m_sans_italic.empty());
		assert(!m_sans_bold_italic.empty());
		assert(!m_condensed.empty());
		assert(!m_condensed_bold.empty());
		assert(!m_condensed_italic.empty());
		assert(!m_condensed_bold_italic.empty());

		if (direction_ == "rtl") {
			m_direction = FontSet::Direction::kRightToLeft;
		} else {
			m_direction = FontSet::Direction::kLeftToRight;
		}
	}

	const std::string& serif() const {return m_serif;}
	const std::string& serif_bold() const {return m_serif_bold;}
	const std::string& serif_italic() const {return m_serif_italic;}
	const std::string& serif_bold_italic() const {return m_serif_bold_italic;}
	const std::string& sans() const {return m_sans;}
	const std::string& sans_bold() const {return m_sans_bold;}
	const std::string& sans_italic() const {return m_sans_italic;}
	const std::string& sans_bold_italic() const {return m_sans_bold_italic;}
	const std::string& condensed() const {return m_condensed;}
	const std::string& condensed_bold() const {return m_condensed_bold;}
	const std::string& condensed_italic() const {return m_condensed_italic;}
	const std::string& condensed_bold_italic() const {return m_condensed_bold_italic;}
	FontSet::Direction direction() {return m_direction;}

private:
	std::string m_serif;
	std::string m_serif_bold;
	std::string m_serif_italic;
	std::string m_serif_bold_italic;
	std::string m_sans;
	std::string m_sans_bold;
	std::string m_sans_italic;
	std::string m_sans_bold_italic;
	std::string m_condensed;
	std::string m_condensed_bold;
	std::string m_condensed_italic;
	std::string m_condensed_bold_italic;
	FontSet::Direction m_direction;
};

}

///Thrown if a commandline parameter is faulty
struct ParameterError : public std::runtime_error {
	explicit ParameterError() : std::runtime_error("") {}
	explicit ParameterError(std::string text)
		: std::runtime_error(text)
	{}
};

// input
struct InputCallback {
	void (*mouse_press)
	(const uint8_t button, // Button number as #defined in SDL_mouse.h.
	 int32_t x, int32_t y);      // The coordinates of the mouse at press time.
	void (*mouse_release)
	(const uint8_t button, // Button number as #defined in SDL_mouse.h.
	 int32_t x, int32_t y);      // The coordinates of the mouse at release time.
	void (*mouse_move)
	(const uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
	void (*key)        (bool down, SDL_Keysym code);
	void (*textinput) (const char * text);
	void (*mouse_wheel) (uint32_t which, int32_t x, int32_t y);
};

/// You know main functions, of course. This is the main struct.
///
/// The oversimplified version: everything else is either game logic or GUI.
///
/// WLAppplication bundles all initialization and shutdown code in one neat
/// package. It also includes all (well, most) system abstractions, notably
/// i18n, input handling, timing, low level networking and graphics setup (the
/// actual graphics work is done by Graphic).
///
// TODO(bedouin): Is the above part about i18n still true?
//
// Equally important, the main event loop is chugging along in this class.
// [not yet but some time in the future \#bedouin8sep2007]
///
/// \par WLApplication is a singleton
///
/// Because of it's special purpose, having more than one WLApplication is
/// useless. So we implement singleton semantics:
/// \li A private(!) static class variable (--> unique for the whole program,
///     although nobody can get at it) the_singleton holds a pointer to the
///     only instance of WLApplication. It is private because it would not be a
///     struct variable otherwise.
/// \li There is no public constructor. If there was, you would be able to
///     create more WLApplications. So constructor access must be encapsulated
///     too.
/// \li The only way to get at the WLApplication object is to call
///     WLApplication::get(), which is static as well. Because of this, get()
///     can access the_singleton even if no WLApplication object has been
///     instantiated yet. get() will \e always give you a valid WLApplication.
///     If one does not exist yet, it will be created.
/// \li A destructor does not make sense. Just make sure you call
///     shutdown_settings() and shutdown_hardware() when you are done - in a
///     sense, it is a destructor without the destruction part ;-)
///
/// These measures \e guarantee that there are no stray WLApplication objects
/// floating around by accident.
///
/// For testing purposes, we can spawn a second process with widelands running
/// in it (see init_double_game()). The fact that WLApplication is a singleton
/// is not touched by this: the processes start out as a byte exact memory
/// copy, so the two instances ca not know (except for fork()'s return value)
/// that they are (or are not) a primary thread. Each WLApplication singleton
/// really *is* a singleton - inside it's own process.
///
/// Forking does not work on windows, but nobody cares enough to investigate.
/// It is only a debugging convenience anyway.
///
///
/// \par The mouse cursor
///
/// SDL can handle a mouse cursor on its own, but only in black and white. That
/// is not sufficient.
///
/// So Widelands must paint its own cursor and hide the system cursor.
///
/// Ordinarily, relative coordinates break down when the cursor leaves the
/// window. This means we have to grab the mouse, then relative coords are
/// always available.
// TODO(unknown): Actually do grab the mouse when it is locked
// TODO(unknown): Graphics are currently not handled by WLApplication, and it is
// non essential for playback anyway. Additionally, we will want several
// rendering backends (software and OpenGL). Maybe the graphics backend loader
// code should be in System, while the actual graphics work is done elsewhere.
// TODO(unknown): Refactor the mainloop
// TODO(unknown): Sensible use of exceptions (goes for whole game)
// TODO(sirver): this class makes no sense for c++ - most of these should be
// stand alone functions.
struct WLApplication {
	static WLApplication * get(int const argc = 0, char const * * argv = nullptr);
	~WLApplication();

	enum GameType {NONE, EDITOR, REPLAY, SCENARIO, LOADGAME, NETWORK, INTERNET};

	void run();

	/// \warning true if an external entity wants us to quit
	bool should_die() const {return m_should_die;}

	int32_t get_time();

	/// Get the state of the current KeyBoard Button
	/// \warning This function doesn't check for dumbness
	bool get_key_state(SDL_Scancode const key) const {return SDL_GetKeyboardState(nullptr)[key];}

	//@{
	void warp_mouse(Point);
	void set_input_grab(bool grab);

	/// The mouse's current coordinates
	Point get_mouse_position() const {return m_mouse_position;}
	//
	/// Find out whether the mouse is currently pressed
	bool is_mouse_pressed() const {return SDL_GetMouseState(nullptr, nullptr); }

	/// Swap left and right mouse key?
	void set_mouse_swap(const bool swap) {m_mouse_swapped = swap;}

	/// Lock the mouse cursor into place (e.g., for scrolling the map)
	void set_mouse_lock(const bool locked) {m_mouse_locked = locked;}
	//@}

	void init_graphics(int32_t w, int32_t h, bool fullscreen, bool opengl);

	/**
	 * Refresh the graphics from the latest options.
	 *
	 * \note See caveats for \ref init_graphics()
	 */
	void refresh_graphics();

	void handle_input(InputCallback const *);

	void mainmenu();
	void mainmenu_tutorial();
	void mainmenu_singleplayer();
	void mainmenu_multiplayer();
	void mainmenu_editor();

	bool new_game();
	bool load_game();
	bool campaign_game();
	void replay();
	static void emergency_save(Widelands::Game &);

	UI::FontSet parse_font_for_locale(const std::string& locale);
	const UI::FontSet& get_fontset() const {return m_fontset;}
	void set_fontset(UI::FontSet fontset) {
		m_fontset = fontset;
	}

protected:
	WLApplication(int argc, char const * const * argv);

	bool poll_event(SDL_Event &);

	bool init_settings();
	void init_language();
	void shutdown_settings();

	bool init_hardware();
	void shutdown_hardware();

	void parse_commandline(int argc, char const * const * argv);
	void handle_commandline_parameters();

	void setup_homedir();

	void cleanup_replays();

	bool redirect_output(std::string path = "");

	/**
	 * The commandline, conveniently repackaged.
	 */
	std::map<std::string, std::string> m_commandline;

	std::string m_filename;

	/// Script to be run after the game was started with --editor,
	/// --scenario or --loadgame.
	std::string m_script_to_run;

	//Log all output to this file if set, otherwise use cout
	std::string m_logfile;

	GameType m_game_type;

	///True if left and right mouse button should be swapped
	bool  m_mouse_swapped;

	/// When apple is involved, the middle mouse button is sometimes send, even
	/// if it wasn't pressed. We try to revert this and this helps.
	bool  m_faking_middle_mouse_button;

	///The current position of the mouse pointer
	Point m_mouse_position;

	///If true, the mouse cursor will \e not move with a mousemotion event:
	///instead, the map will be scrolled
	bool  m_mouse_locked;

	///If the mouse needs to be moved in warp_mouse(), this Point is
	///used to cancel the resulting SDL_MouseMotionEvent.
	Point m_mouse_compensate_warp;

	///true if an external entity wants us to quit
	bool   m_should_die;

	//do we want to search the default places for widelands installs
	bool   m_use_default_datadir;
	std::string m_homedir;

	/// flag indicating if stdout and stderr have been redirected
	bool m_redirected_stdio;
private:
	///Holds this process' one and only instance of WLApplication, if it was
	///created already. nullptr otherwise.
	///\note This is private on purpose. Read the class documentation.
	static WLApplication * the_singleton;

	void _handle_mousebutton(SDL_Event &, InputCallback const *);

	UI::FontSet m_fontset;

};

#endif  // end of include guard: WL_WLAPPLICATION_H
