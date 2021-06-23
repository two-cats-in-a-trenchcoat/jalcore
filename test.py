from PIL import Image

value = ""

file = open("core.bin", "r")
value = file.read()
file.close()

cmap = {'0': (255,255,255),
        '1': (0,0,0)}

data = [cmap[letter] for letter in value]
img = Image.new('RGB', (8, len(value)//8), "white")
img.putdata(data)
img.show()
