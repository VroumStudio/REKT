How to build the Wwise Authoring API C++ sample on OSX:

1. Go to the root of the source (where this file is) in a terminal.
2. Modify TestClient.cpp to call Connect with the IP of your computer running Wwise Authoring. By default, it is localhost.
3. ./mac_build.sh
4. ./mac_run.sh

You can edit mac_build.sh to modify the configuration. The following configuration are available in the xcode4 project:

Debug
Profile
Release
