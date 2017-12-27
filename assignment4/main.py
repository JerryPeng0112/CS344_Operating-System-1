import random
import string
from random import randint
strArr = []
inputStr = ""
a = ""
fileName = ""
x = ""
for j in range(0, 3):
    fileName = "file"
    inputStr = ""
    for i in range(0, 10):
        a = random.choice(string.ascii_lowercase)
        inputStr+=a
    inputStr+='\n'
    fileName+=str(j)
    f = open(fileName, 'w+')
    f.write(inputStr)
    print(inputStr, end = "")
num1 = randint(1, 42)
num2 = randint(1, 42)
print(num1)
print(num2)
print(num1*num2)
