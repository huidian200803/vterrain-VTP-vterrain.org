#!/bin/sh

TERRAINAPPS=/cygdrive/c/VTP/TerrainApps

lftp -c "open sftp://vterrain@cyclone.he.net; \
		 cd public_html/Doc; \
		 lcd $TERRAINAPPS; \
		 mirror -R --verbose --continue --no-perms --exclude .svn BExtractor/Docs BExtractor; \
		 mirror -R --verbose --continue --no-perms --exclude .svn --exclude _private --exclude _vti_cnf --exclude _vti_pvt Enviro/Docs Enviro; \
		 mirror -R --verbose --continue --no-perms --exclude .svn VTBuilder/Docs VTBuilder; \
		 mirror -R --verbose --continue --no-perms --exclude .svn CManager/Docs CManager; \
		 put docs.css"
