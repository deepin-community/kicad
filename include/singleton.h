/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright The KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KICAD_SINGLETON_H
#define KICAD_SINGLETON_H

#include <advanced_config.h>

class GL_CONTEXT_MANAGER;
namespace BS
{
class thread_pool;
}

class KICAD_SINGLETON
{
public:
    KICAD_SINGLETON(){};

    ~KICAD_SINGLETON();


    void Init();

    BS::thread_pool* m_ThreadPool;
    GL_CONTEXT_MANAGER* m_GLContextManager;
};


#endif // KICAD_SINGLETON_H