build/chat-client.exe: src/**/*.c src/**/*.h src/*.c
	clang -Lbuild -lui -lcurl-x64 -llibcJSON \
		-lws2_32 -lpthread -O2 $(filter %.c,$^) -o $@
