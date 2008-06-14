GRE_HOME=/c/xulrunner-sdk/bin

MOZ_LDPATHS=\
	$(GRE_HOME)/../lib \
	$(GRE_HOME)/../../profile/dirserviceprovider/standalone

MOZ_LIBS=xpcomglue kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 uuid 

MOZ_LDFLAGS=$(addprefix -L, $(MOZ_LDPATHS)) $(addprefix -l, $(MOZ_LIBS))

GTK_CFLAGS=`pkg-config --cflags gtk+-2.0`
GTK_LDFLAGS=`pkg-config --libs gtk+-2.0`


%.o:%.c
	gcc -o $@ -c $< -I../ -I../../common  $(GTK_CFLAGS)

test:main.o
	gcc -o $@ $< $(MOZ_LDFLAGS) $(GTK_LDFLAGS) ../mozarea.lib


clean:
	rm -f test.exe *~ *.o
