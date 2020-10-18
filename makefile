all: discord.so

discord.so:
	cc -Wall mpvdiscord.c libdiscord-rpc.a -shared -o discord.so

clean:
	rm discord.so
