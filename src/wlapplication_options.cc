/*
 * Copyright (C) 2012-2019 by the Widelands Development Team
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

#include "wlapplication_options.h"

#include "base/log.h"
#include "io/filesystem/disk_filesystem.h"
#include "io/profile.h"
#include "logic/filesystem_constants.h"
#include "wlapplication.h"

Profile g_options(Profile::err_log);

void check_config_used() {
    g_options.check_used();
}

Section& get_config_section() {
    return g_options.pull_section("global");
}

Section& get_config_section(const std::string& section) {
    return g_options.pull_section(section.c_str());
}

Section* get_config_section_ptr(const std::string& section) {
    return g_options.get_section(section.c_str());
}

bool get_config_bool(const std::string& name, const bool dflt) {
    return g_options.pull_section("global").get_bool(name.c_str(), dflt);
}

bool get_config_bool(const std::string& section,
                     const std::string& name,
                     const bool dflt) {
    return g_options.pull_section(section.c_str()).get_bool(name.c_str(), dflt);
}

int32_t get_config_int(const std::string& name, const int32_t dflt) {
    return g_options.pull_section("global").get_int(name.c_str(), dflt);
}

int32_t get_config_int(const std::string& section,
                       const std::string& name,
                       const int32_t dflt) {
    return g_options.pull_section(section.c_str()).get_int(name.c_str(), dflt);
}

uint32_t get_config_natural(const std::string& name, const uint32_t dflt) {
    return g_options.pull_section("global").get_natural(name.c_str(), dflt);
}

uint32_t get_config_natural(const std::string& section,
                            const std::string& name,
                            uint32_t dflt) {
    return g_options.pull_section(section.c_str()).get_natural(name.c_str(), dflt);
}

std::string get_config_string(const std::string& name,
                              const std::string& dflt) {
    return g_options.pull_section("global").get_string(name.c_str(), dflt.c_str());
}

std::string get_config_string(const std::string& section,
                              const std::string& name,
                              const std::string& dflt) {
    return g_options.pull_section(section.c_str()).get_string(name.c_str(), dflt.c_str());
}

Section& get_config_safe_section() {
    return g_options.get_safe_section("global");
}

Section& get_config_safe_section(const std::string& section) {
    return g_options.get_safe_section(section.c_str());
}

void set_config_bool(const std::string& name, const bool value) {
    g_options.pull_section("global").set_bool(name.c_str(), value);
}

void set_config_bool(const std::string& section,
                     const std::string& name,
                     const bool value) {
    g_options.pull_section(section.c_str()).set_bool(name.c_str(), value);
}

void set_config_int(const std::string& name, int32_t value) {
    g_options.pull_section("global").set_int(name.c_str(), value);
}

void set_config_int(const std::string& section,
                    const std::string& name,
                    const int32_t value) {
    g_options.pull_section(section.c_str()).set_int(name.c_str(), value);
}

void set_config_string(const std::string& name, const std::string& value) {
    set_config_string(name.c_str(), value.c_str());
}

void set_config_string(const std::string& section,
                       const std::string& name,
                       const std::string& value) {
    set_config_string(section.c_str(), name.c_str(), value.c_str());
}

void read_config(WLApplication* wlapplication) {
#ifdef USE_XDG
    RealFSImpl dir(wlapplication->get_userconfigdir());
    dir.ensure_directory_exists(".");
    log("Set configuration file: %s/%s\n",
        wlapplication->get_userconfigdir().c_str(),
        kConfigFile.c_str());
    g_options.read(kConfigFile.c_str(), "global", dir);
#else
    g_options.read(kConfigFile.c_str(), "global");
#endif
}

void write_config(WLApplication* wlapplication) {
    try {  //  overwrite the old config file
#ifdef USE_XDG
            RealFSImpl dir(wlapplication->get_userconfigdir());
            g_options.write(kConfigFile.c_str(), true, dir);
#else
            g_options.write(kConfigFile.c_str(), true);
#endif
    } catch (const std::exception& e) {
        log("WARNING: could not save configuration: %s\n", e.what());
    } catch (...) {
        log("WARNING: could not save configuration");
    }
}
