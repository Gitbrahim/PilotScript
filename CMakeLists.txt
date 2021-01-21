#
# Copyright (c) 2018 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
#

cmake_minimum_required(VERSION 3.8.2)

include(../../../cmake/boilerplate.cmake NO_POLICY_SCOPE)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(mqtt-simple)


# NORDIC SDK APP START
target_sources(app PRIVATE src/main.c)


# ELVA LIBRARIES START
zephyr_include_directories(src)

include_directories(src/LS013B7DH05_library)
target_sources(app PRIVATE src/LS013B7DH05_library/LS013B7DH05_library.c)

include_directories(src/MT15_library)
target_sources(app PRIVATE src/MT15_library/MT15_library.c)

include_directories(src/elva_initializer)
target_sources(app PRIVATE src/elva_initializer/elva_initializer.c)

include_directories(src/DRV2605L_library)
target_sources(app PRIVATE src/DRV2605L_library/DRV2605L_library.c)

include_directories(src/BQ27441_library)
target_sources(app PRIVATE src/BQ27441_library/BQ27441_library.c)

include_directories(src/medication_reminder)
target_sources(app PRIVATE src/medication_reminder/medication_reminder.c)

include_directories(src/ui_windows)
target_sources(app PRIVATE src/ui_windows/ui_windows.c)


# ELVA LIBRARIES END

# NORDIC SDK APP END
