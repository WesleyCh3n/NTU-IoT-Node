docker run -it --rm --privileged=true -w /home/ -v `pwd`:/home/ -v /opt/vc:/opt/vc --device=/dev/vchiq --device=/dev/vcsm cpp-slim bash
