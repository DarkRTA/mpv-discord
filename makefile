all: discord.so

discord.so:
	cc mpvdiscord.c libdiscord-rpc.a -shared -o discord.so

clean:
	rm discord.so
