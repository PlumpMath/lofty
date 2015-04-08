﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015
Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with Abaclade. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#include <abaclade/app.hxx>
#include <abaclade/coroutine.hxx>
#include <abaclade/net.hxx>

using namespace abc;


////////////////////////////////////////////////////////////////////////////////////////////////////
// echo_server_app

class echo_server_app : public app {
public:
   /*! Main function of the program.

   @param vsArgs
      Arguments that were provided to this program via command line.
   @return
      Return value of this program.
   */
   virtual int main(collections::mvector<istr const> const & vsArgs) override {
      ABC_TRACE_FUNC(this, vsArgs);

      auto & pcorosched = this_thread::attach_coroutine_scheduler();

      // Schedule a TCP server. To connect to it, use: socat - TCP4:127.0.0.1:9082
      pcorosched->add(coroutine([this] () -> void {
         ABC_TRACE_FUNC(this);

         static std::uint16_t const sc_iPort = 9082;
         io::text::stdout->print(ABC_SL("server: starting, listening on port {}\n"), sc_iPort);
         net::tcp_server server(ABC_SL("*"), sc_iPort);
         for (;;) {
            io::text::stdout->write_line(ABC_SL("server: accepting"));
            // This will cause a context switch if no connections are ready to be established.
            auto pconn(server.accept());

            io::text::stdout->write_line(ABC_SL("server: connection established"));

            // Add a coroutine that will echo every line sent over the newly-established connection.
            this_thread::get_coroutine_scheduler()->add(coroutine([this, pconn] () -> void {
               ABC_TRACE_FUNC(this, pconn);

               io::text::stdout->write_line(ABC_SL("responder: starting"));

               // Create text-mode reader and writer for the connection’s socket.
               auto ptr(io::text::make_reader(pconn->socket()));
               auto ptw(io::text::make_writer(pconn->socket()));

               // Read lines from the socket, writing them back to it (echo).
               ABC_FOR_EACH(auto & sLine, ptr->lines()) {
                  ptw->write_line(sLine);
                  ptw->flush();
               }
               io::text::stdout->write_line(ABC_SL("responder: terminating"));
            }));
         }
         io::text::stdout->write_line(ABC_SL("server: terminating"));
      }));

      // Switch this thread to run coroutines, until they all terminate.
      pcorosched->run();
      // Execution resumes here, after all coroutines have terminated.
      io::text::stdout->write_line(ABC_SL("main: terminating"));
      return 0;
   }
};

ABC_APP_CLASS(echo_server_app)
