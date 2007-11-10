//This class is used to load and execute weapon files

import java.io.*;
import java.util.*;

public class weaponInfo{
	String path;
	String current = "Nothing";
	//These arraylists are used to store various script commands to their respective group e.g. [General], [Projectile] etc.
	ArrayList general = new ArrayList();
	ArrayList projectile = new ArrayList();
	ArrayList beam = new ArrayList();

	//Stores the line numbers for each statement
	ArrayList generalnum = new ArrayList();
	ArrayList projectilenum = new ArrayList();
	ArrayList beamnum = new ArrayList();

	projectileInfo projinfo;
	String path2;
	String fname;
	String fname2;
	documentIndenter docindent;
	main par1;
	int linenum = 1;
	boolean exists = true;

	//General Variables
	String name = "";
	String wclass = "null";
	String recoil = "0";
	String recharge = "0";
	String rounds = "0";
	String rof = "0";
	String sound = "";
	String reloadsound = "Reload.ogg";
	boolean pctrl = false;
	boolean lasersight = false;

	//Beam Variables
	String damage = "0";
	String playerdamage = "0";
	String colour = "0,0,0";
	String length = "0";
	boolean usebeam = false;
	String bxoffset = "0";
	String byoffset = "0";
	String bangle = "me.owner.angle";

	//Projectile Variables
	String amount  = "0";
	String speed = "0";
	String speedvar = "0";
	String spread = "0";
	String proj = "";
	String xoffset = "0";
	String yoffset = "0";
	String angle = "me.owner.angle";
	boolean useangle = false;




	public weaponInfo(String path,String path2,String fname, mainReader par, main par1){
		this.path = path;
		this.path2 = path2;
		this.fname = fname;
		this.par1 = par1;
		fname2 = fname;
		//Load the weapon file
		exec(path);

		if(exists){
			//Sort the commands into variables
			sortGeneral();
			sortBeam();
			sortProjectile();
			luaRename();
			save();
			docindent = new documentIndenter(path2+"\\lua\\"+fname+".lua");

			((mainReader)par).returnname(name);
		}
	}

	public void luaRename(){
		if(proj.length() > 3){
			int ts = proj.length()-4;
			String temp = proj.substring(0,ts);
			proj = temp+".lua";

		}
	}


	public void exec(String flag){
		File file9 = new File(flag);
		BufferedReader br1, br2,br3, br4;
		try{
			br1 = new BufferedReader(new InputStreamReader(new FileInputStream(file9)));
			br2 = new BufferedReader(new InputStreamReader(new FileInputStream(file9)));
			br3 = new BufferedReader(new InputStreamReader(new FileInputStream(file9)));
			br4 = new BufferedReader(new InputStreamReader(new FileInputStream(file9)));
		}
		catch(FileNotFoundException ex){
			//System.out.println("Error: "+path+" not found");
			((main)par1).addError(fname2+".txt not found");
			exists = false;
			//((main)par1).exit();
			return;
		}

		int i = 0;

		String s = "";
		int ssize = 0;
		String s1 = "";
		String s2 = "";
		String temp;
		try{
			while(br2.readLine() != null){
				if(!br3.readLine().startsWith("#")){
					if(br4.readLine().startsWith("[")){
						String ttt = br1.readLine();
						String ttt2 = ttt.trim();
						getSection(ttt2);
					}else{
						String ttt = br1.readLine();
						String ttt2 = ttt.trim();
						addCommand(ttt2);
					}
				}else{
					temp = br4.readLine();
					temp = br1.readLine();
				}
				i++;
				linenum++;

			}

		}
		catch(IOException ex){
			return;
		}

	}


	//Add Script commands to the currently active group
	public void addCommand(String com){
		if(current.equalsIgnoreCase("general")){

			if(com.length() > 1){
				general.add(com);
				generalnum.add(""+linenum);
			}

		}
		else if(current.equalsIgnoreCase("projectile")){
			if(com.length() > 1){
				projectile.add(com);
				projectilenum.add(""+linenum);
			}
		}

		else if(current.equalsIgnoreCase("beam")){
			if(com.length() > 1){
				beam.add(com);
				beamnum.add(""+linenum);
			}
		}else if(!getCommand(com).trim().equalsIgnoreCase("")){
			((main)par1).addWarning("Statement out of place: \""+getCommand(com)+"\" at line "+linenum+" in "+fname2+".txt");
		}
	}

	//Method used to change the group that the commands are being added to
	public void getSection(String sect){

		int length = sect.length();
		String msect = sect.substring(1,length-1);

		if(msect.equalsIgnoreCase("general"))
			current = "general";
		else if(msect.equalsIgnoreCase("projectile"))
			current = "projectile";
		else if(msect.equalsIgnoreCase("beam"))
			current = "beam";
		else
			((main)par1).addWarning("Invalid Section: \""+msect+"\" at line "+linenum+" in "+fname2+".txt");

	}

	public void sortGeneral(){
		for(int i=0;i<general.size();i++){
			String code = (String)general.get(i);
			//System.out.println(i+", "+general.size()+", "+code);
			if(getCommand(code).equalsIgnoreCase("name")){

				name = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("class")){

				wclass = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("recoil")){
				recoil = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("recharge")){
				recharge = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("rounds")){
				rounds = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("drain")){
				String ttemp2 = getVar(code);
				double temp = Double.parseDouble(ttemp2);
				int temp2 = 0;
				double tnum2 = 0.0;
				double tnum = 0.0;
				double t = 101.0;
				do{

					if(tnum < 101){
						tnum2++;
					}else{
						break;
					}
					tnum = tnum+temp;

				}while(temp2 == 0);
				rounds = ""+tnum2;
			}
			else if(getCommand(code).equalsIgnoreCase("ROF")){
				rof = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("Sound")){
				sound = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("ReloadSound")){
				reloadsound = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("Playercontrolled")){
				String temp = getVar(code);
				if(temp.equalsIgnoreCase("true"))
					pctrl = true;
				else
					pctrl = false;
			}
			else if(getCommand(code).equalsIgnoreCase("LaserSight")){
				String temp = getVar(code);
				if(temp.equalsIgnoreCase("true"))
					lasersight = true;
				else
					lasersight = false;
			}else{
				((main)par1).addWarning("Invalid statement: \""+getCommand(code)+"\" at line "+(String)generalnum.get(i)+" in "+fname2+".txt");
			}

		}

	}

	public void sortBeam(){
		for(int i=0;i<beam.size();i++){
			String code = (String)beam.get(i);
			//System.out.println(i+", "+beam.size()+", "+code);
			if(getCommand(code).equalsIgnoreCase("damage")){

				damage = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("playerdamage")){
				playerdamage = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("colour")){
				colour = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("length")){
				length = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("xoffset")){
				bxoffset = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("yoffset")){
				byoffset = getVar(code);
			}else{
				((main)par1).addWarning("Invalid statement: \""+getCommand(code)+"\" at line "+(String)beamnum.get(i)+" in "+fname2+".txt");
			}

		}

	}

	public void sortProjectile(){
		for(int i=0;i<projectile.size();i++){
			String code = (String)projectile.get(i);

			if(getCommand(code).equalsIgnoreCase("Amount")){
				amount = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("Speed")){
				speed = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("SpeedVar")){
				speedvar = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("Spread")){
				spread = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("Projectile")){
				proj = getVar(code);
				System.out.println("Compiling: "+proj);


				projinfo = new projectileInfo(path2+"//"+proj, path2,getFname(proj),par1);

			}
			else if(getCommand(code).equalsIgnoreCase("Angle")){
				angle = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("Useangle")){
				String temp = getVar(code);
				if(temp.equalsIgnoreCase("true"))
					useangle = true;
				else
					useangle = false;
			}else if(getCommand(code).equalsIgnoreCase("Xoffset")){
				xoffset = getVar(code);
			}else if(getCommand(code).equalsIgnoreCase("Yoffset")){
				yoffset = getVar(code);
			}else{
				((main)par1).addWarning("Invalid statement: \""+getCommand(code)+"\" at line "+(String)projectilenum.get(i)+" in "+fname2+".txt");
			}
		}

	}

	public String getFname(String fname1){
		int ilength = fname1.length()-4;
		String fname2 = fname1.substring(0,ilength);
		return fname2;

	}

	public void saveGeneral(PrintWriter printwriter){
		//First things
		//printwriter.println("include(\"Common.lua\")");
		//printwriter.println("global reloadtime = 0");

		//initialize
		printwriter.println("function initialize()");
		String nclass = gettheClass();
		printwriter.println("me.class = "+nclass);
		printwriter.println("me.recoil = "+recoil);
		double tempd = Double.parseDouble(rounds);
		printwriter.println("me.rounds = "+(int)tempd);
		printwriter.println("me.recharge = "+recharge);
		printwriter.println("me.rof = "+rof);
		if(lasersight){
			printwriter.println("me.lasersight = true");
		}

		//callbacks
		printwriter.println("callbacks.realoding = \"reloading\"");
		printwriter.println("callbacks.onfire = \"onfire\"");
		printwriter.println("callbacks.oninput = \"oninput\"");
		printwriter.println("end");

		//reloading
		printwriter.println("function reloading()");
		printwriter.println("reloadtime = reloadtime + lx.deltatime");
		printwriter.println("if reloadtime > 1 then");
		printwriter.println("sample(\""+reloadsound+"\"):play()");
		printwriter.println("me.ammo = me.ammo + 1");
		printwriter.println("reloadtime = 0");
		printwriter.println("end");
		printwriter.println("end");

		//onfire
		printwriter.println("function onfire()");
		printwriter.println("if me.ammo > 0 then");
		if(!usebeam){
			if(!sound.trim().equalsIgnoreCase(""))
				printwriter.println("sample(\""+sound+"\"):play()");
			if(!speedvar.trim().equalsIgnoreCase("")){
				printwriter.println("me:spawn(\""+proj+"\", "+amount+", "+xoffset+", "+yoffset+", "+angle+", "+speed+", "+spread+")");
			}else{
				printwriter.println("me:spawn(\""+proj+"\", "+amount+", "+xoffset+", "+yoffset+", "+angle+",  vector("+speed+","+speedvar+"), "+spread+")");
			}
		}else{
			printwriter.println("me:spawnbeam("+bxoffset+", "+byoffset+", "+bangle+", colour("+colour+"), "+length+", "+damage+")");
		}
		printwriter.println("me.ammo = me.ammo - 1");
		if(pctrl){
			printwriter.println("if me.inputfocus == false then");
			printwriter.println("me:focusinput()");
			printwriter.println("end");
		}
		printwriter.println("end");
		printwriter.println("end");

		//controll stuff

		if(pctrl){
			printwriter.println("function oninput(input)");
			//printwriter.println("result = false");
			printwriter.println("if input.movement.x != 0 | input.movement.y != 0 then");
			printwriter.println("for i = 0, i > me.projectiles.count, i++");
			printwriter.println("me.projectiles.get(i):move(input.movement)");
			printwriter.println("end");
			printwriter.println("end");
			//printwriter.println("return result");
			printwriter.println("end");

		}



	}

	public String gettheClass(){
		String sclass = "";
		if(wclass.equalsIgnoreCase("WCL_AUTOMATIC"))
			return "weapons:automatic";
		else if(wclass.equalsIgnoreCase("WCL_CLOSERANGE"))
			return "weapons:closerange";
		else if(wclass.equalsIgnoreCase("WCL_GRENADE"))
			return "weapons:grenade";
		else if(wclass.equalsIgnoreCase("WCL_MISSILE"))
			return "weapons:missile";
		else if(wclass.equalsIgnoreCase("WCL_POWERGUN"))
			return "weapons:powerweapon";
		else if(wclass.equalsIgnoreCase("WCL_BEAM")){
			usebeam = true;
			return "weapons:laser";
		}
		//((main)par1).addWarning("Invalid weapon class: \""+wclass+"\" at line "+linenum+" in "+fname2+".txt");
		return "weapons:none";

	}


	public void save(){
			File file9;
			File file1;
			//System.out.println(path2+"\\lua\\"+fname+".lua");
			file9 = new File(path2+"\\lua\\"+fname+".lua");
			file1 = new File(path2+"\\lua");
			file1.mkdirs();

			PrintWriter printwriter;
			File file = file9;
			try{
				printwriter = new PrintWriter(new BufferedOutputStream(new FileOutputStream(file)));
			}
			catch(IOException ioexception){
				System.out.println(ioexception);
				System.out.println(file);
				System.out.println(file.canRead());
				System.out.println(file.canWrite());
				return;
			}

			String t1 = "";
			String t2 = "";
			String t3 = "";
			printwriter.println("-- Created by the LXE Script Basic Compiler");
			saveGeneral(printwriter);
			//int g = fc.size();
			//for(int i=0;i<g;i++){
				//printwriter.println(encr.encrypt((String)fc.get(i)));
			//}









			printwriter.close();

	}




	//Get's the command form a line of script
	public String getCommand(String command){
		String com = command.trim();

		//find the =;
		boolean found = false;

		int n1=0;
		int n2=0;

			for(int i=0;i<com.length();i++){
				if(com.substring(i,i+1).equalsIgnoreCase("=")){
					n1 = i;
					n2 = i+1;
					found = true;
					break;
				}
			}


		if(found){
			String com2 = com.substring(0,n1-1);
			return com2;
		}
		return command;



	}
	//Get's the variable from a command
	public String getVar(String command){
			String com = command.trim();
			//find the =;
			int n1=0;
			int n2=0;
			for(int i=0;i<com.length();i++){
				if(com.substring(i,i+1).equalsIgnoreCase("=")){
					n1 = i;
					n2 = i+1;
					break;
				}
			}

			String com2 = com.substring(n2+1,com.length());
			return com2;



	}


}