if __name__ != '__main__':
	raise RuntimeError("Must be run as a script")

fname = input("Enter filename: ").strip()

with open(fname, "wb") as fd:
	while True:
		address = int(input("Enter address: "), 16)
		if not address:
			break
		print("Enter hex:")

		hexbin = b""
		while True:
			hexin = input().strip()
			if not hexin:
				break
			hexbin += int(hexin, 16).to_bytes(len(hexin) // 2, byteorder='little')

		fd.seek(address)
		fd.write(hexbin)

