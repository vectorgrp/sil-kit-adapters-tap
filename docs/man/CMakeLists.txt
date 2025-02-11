# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

if (UNIX)
    include(GNUInstallDirs)
    install(
        FILES
            sil-kit-adapter-tap.1
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1/
        COMPONENT docs
    )
endif()