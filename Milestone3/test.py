if __name__=="__main__":
	fp = open("testing", "w+")
	for i in range(10000):
		fp.write("hello how are you ? " + str(i) + "\n")
	fp.close()
