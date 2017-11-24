
This application demonstrates very simple use of the vtlib library,
to load and render a single terrain.

By default, it will look for the file "crater_0513.bt" in the folder
wxSimple/Data/Elevation.  You should copy the file to that location
from the folder Enviro/Data/Elevation.

You can edit the file Data/Simple.ini to change the elevation file
or any other terrain parameter.

It uses wxWindows for the application framework, and assumes that you
have the rest of the VTP libraries on your machine.

See the function CreateScene() in the file app.cpp for how to set up
a very simple scene.

