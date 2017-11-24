#!/bin/sh

TERRAINSDK=/cygdrive/c/VTP/TerrainSDK

lftp -c "open sftp://vterrain@cyclone.he.net; \
		 cd public_html/Doc; \
		 lcd $TERRAINSDK/Doc; \
		 mirror -R --verbose --continue --no-perms vtdata vtdata; \
		 mirror -R --verbose --continue --no-perms vtlib vtlib; \
		 mirror -R --verbose --continue --no-perms enviro enviro"
