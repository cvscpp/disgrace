/*
 * Disgrace - Digital Audio Workstation
 * Copyright (C) 2025  Miroslav Shaltev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>

namespace disgrace_ns {

inline std::string app_name() {
#ifdef PACKAGE_NAME
    return PACKAGE_NAME;
#else
    return "disgrace";
#endif
}

inline std::string app_version() {
#ifdef PACKAGE_VERSION
    return PACKAGE_VERSION;
#else
    return "0.0.0";
#endif
}

inline std::string app_display_name() {
    return "Disgrace";
}

inline std::string app_display_name_with_version() {
    return app_display_name() + " " + app_version();
}

inline std::string app_license_summary() {
    return "GNU General Public License v3 or later";
}

inline std::string app_disclaimer_summary() {
    return "This program is distributed in the hope that it will be useful, "
           "but WITHOUT ANY WARRANTY; without even the implied warranty of "
           "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for details.";
}

} // namespace disgrace_ns
