build/chat-client.exe: src/**/*.c src/**/*.h src/*.c
#	clang -Lbuild -lcurl-x64 -lws2_32 -llibcJSON -lpthread -lui \
#		-Wl,--subsystem,windows -O2 $(filter %.c,$^) -o $@
	clang -Lbuild -lcurl-x64 -lws2_32 -llibcJSON -lpthread -lui \
		-fsanitize=address -O0 -g $(filter %.c,$^) -o $@
