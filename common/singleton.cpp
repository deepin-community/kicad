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

#include <singleton.h>
#include <bs_thread_pool.hpp>
#include <gal/opengl/gl_context_mgr.h>


KICAD_SINGLETON::~KICAD_SINGLETON()
{
    // This will wait for all threads to finish and then join them to the main thread
    delete m_ThreadPool;

    m_ThreadPool = nullptr;

    m_GLContextManager->DeleteAll();
    delete m_GLContextManager;
    m_GLContextManager = nullptr;
}


void KICAD_SINGLETON::Init()
{
    int num_threads = std::max( 0, ADVANCED_CFG::GetCfg().m_MaximumThreads );
    m_ThreadPool = new BS::thread_pool( num_threads );
    m_GLContextManager = new GL_CONTEXT_MANAGER();
}