build/chat-client.exe: src/**/*.c src/**/*.h src/*.c
	clang -Lbuild -lui -lcurl-x64 -llibcJSON \
		-lpthread -O2 $(filter %.c,$^) -o $@
