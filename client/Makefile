build/chat-client.exe: deps/cJSON/cJSON.c src/*.c src/*.h
	clang -Lbuild -lui -lcurl-x64 -lpthread -O2 $(filter %.c,$^) -o $@
