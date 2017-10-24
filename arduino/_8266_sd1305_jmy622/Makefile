#
# Very ugly makefile, that for now will simply invoke arduino and build a new image in a build/ folder
# probably some extras should be added, regarding what is needed and requirements
#

all: make show

make:
	arduino --verify _8266_sd1305_jmy622.ino --board esp8266:esp8266:huzzah:CpuFrequency=80,UploadSpeed=115200,FlashSize=4M3M --pref build.path=./build/

show:
	ls -la build/

clean:
	rm -r build/
