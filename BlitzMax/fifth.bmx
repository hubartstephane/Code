
Graphics 800, 600

HideMouse


Confirm("Hello")
Proceed("Hello 2")

RequestFile("toto", "truc:png")

Repeat

	

	Cls
	
	Local x = MouseX()
	Local y = MouseY()
	
	DrawRect x, y, 50, 50
	
	
	Flip

Until KeyDown(Key_Escape)

