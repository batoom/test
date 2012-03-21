import sun.awt.DisplayChangedListener;



class HelloWorld {
    public int num;
    public String name;


	public HelloWorld(){
		System.out.println("clash construct");
	}

    public HelloWorld(int num, String name){
        this.num = num;
        this.name = name;
    }

    public void display() {
        System.out.println(name);
        System.out.println(num);
    }
	public static void main(String args[])
	{
		HelloWorld hello = new HelloWorld(3,"wangwei");
		hello.display();
		System.out.println(hello.name);
		System.out.println(hello.num);
	}

}
