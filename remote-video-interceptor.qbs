/*

  Notes: To be able to answer to the requested test in a confortable delay
  I choosed to write it with Qt, in this will I should be able to make a good prototype.

  In a company I may choose to do it as a first step called a Proof Of Concept, but for a
  long term solution (5 to 10 years) I certainly go away to Qt to do everything from scratch
  to be able to master all piece of the final software. Because my experience tends to show me
  that Qt can have a development cycle that impacts ours.

  Plus if we want to maximize the performance of a server it certainly requiere a fine tuning
  pretty specific to OS. As far as I know on Windows we should want to use IOCompletion port
  with asynchronous mecanisms to be able to maximize the number of concurrent requests.

  On the protocol side it depends of what we can do, if it is feasible a in house specific protocol
  can be better for low latency constraints, but if we have to use something more standard and widely
  adopted we should look to LHLS (Low Latency HLS) that is based on HTTP2 to reduce overhead of
  establishing repeated HTTP/TCP connections.

  PS: I am using some QStringRef to avoid allocations and copies, this is essentially usefull for the header
  that we have to parse. It force us to keep a reference on the original QString.
  Passing return value as ref parameters also avoid copies.

  */

Project
{
    name: "remote-video-interceptor"

    CppApplication
    {
        name: "remote-video-interceptor"

        consoleApplication: true

        files: [
            "main.cpp",
            "server.cpp",
            "server.hpp",
        ]

        cpp.cxxLanguageVersion: ["c++17"]

        cpp.dynamicLibraries: [
        ]

        Depends {name: "Qt.core" }
        Depends {name: "Qt.network" }
    }
}
