print("Generated frac table!")

for x in range(-1, -24, -1):
    number = (pow(2, x)); 
    print(f"{number},"); 


print("Generated expn table!")

for x in range(0, 256):
    print(x, x-127)
    number = (pow(2, x-127)); 
    print(f"{number},"); 
    