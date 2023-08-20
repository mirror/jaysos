
package Applications;

// library inclusions
import java.lang.*;
import waba.sys.*;
import waba.sys.VmShell;


class Greeter {

  String _name;

  public Greeter(String name) {
      _name = name;

  }

  public void greet(String greeting) {
      VmShell.println(_name + " says \"" + greeting + "\"");

  }


}

class Test1 {

    public static void main (String[] args) {

	String string = "platform = " + Vm.getPlatform() + "\n";
	string += "user = " + Vm.getUserName();

	VmShell.println(string);

	VmShell.println("uptime = " + Vm.getTimeStamp());

	VmShell.println("sleep 2 secs...");
	Vm.sleep(2000);
	VmShell.println("done!");


        Greeter g1 = new Greeter("justin");
        g1.greet("fuck you asshole");

        Greeter g2 = new Greeter("not justin");
        g2.greet("hello");

    }


}
