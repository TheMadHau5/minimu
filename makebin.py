if __name__ != '__main__':
	raise RuntimeError("Must be run as a script")

fname = input("Enter filename: ").strip()

with open(fname, "wb") as fd:
	while True:
		address = input("Enter address: ").strip()
		if not address:
			break
		print("Enter hex:")

		endianness='little'
		if address[0].lower() == 'b':
			endianness = 'big'
			address = address[1:]
		elif address[0].lower() == 'l':
			endianness = 'little'
			address = address[1:]
		address = int(address, 16)

		hexbin = b""
		while True:
			hexin = input().strip()
			if not hexin:
				break
			hexbin += int(hexin, 16).to_bytes(len(hexin) // 2, byteorder=endianness)

		fd.seek(address)
		fd.write(hexbin)

