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
