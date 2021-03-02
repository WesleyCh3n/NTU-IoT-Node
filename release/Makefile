
VERSION := $(V)
RELEASEFILES := ../ntu-iot-node ../model ../upload.sh ../19-01.tsv
RELEASETAR := ntu-iot-node_$(VERSION)_linux_arm32v7.tar.xz

all: GITUPLOAD

GITUPLOAD: $(RELEASETAR)
	@echo "Create new release: $(VERSION)"
	github-release release \
		-u WesleyCh3n \
		-r ntu-iot-node \
		-t v$(VERSION) \
		-n v$(VERSION) \
		-d "$(D)"
	@echo "Uploading $(RELEASETAR) to $(VERSION)"
	github-release upload \
		-u WesleyCh3n \
		-r ntu-iot-node \
		-t v$(VERSION) \
		-n $(RELEASETAR) \
		-f $(RELEASETAR)

$(RELEASETAR): $(RELEASEFILES)
	@echo "Tar-ing $(RELEASETAR)"
	tar cJf $(RELEASETAR) $(RELEASEFILES)
