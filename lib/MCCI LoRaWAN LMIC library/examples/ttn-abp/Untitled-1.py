hex_string = "0f 6e ea 3a f6 da 49 1e dd c7 75 f7 54 d0 bf 8e"
hex_list = [int(x, 16) for x in hex_string.split()]
hex_array = ", ".join([f"0x{value:02X}" for value in hex_list])

print(hex_array)
