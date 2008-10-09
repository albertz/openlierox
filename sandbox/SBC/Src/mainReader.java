//This class is used to load and execute main.txt

import java.io.*;
import java.util.*;

public class mainReader{
	String path;
	String current = "Nothing";
	main par;
	int linenum = 1;
	//These arraylists are used to store various script commands to their respective group e.g. [General], [Projectile] etc.
	ArrayList general = new ArrayList();
	ArrayList weapons = new ArrayList();

	ArrayList weaponnames = new ArrayList();
	ArrayList weapon = new ArrayList();

	ArrayList sfx = new ArrayList();

	ArrayList gfx = new ArrayList();

	ArrayList ninjarope = new ArrayList();
	ArrayList worm = new ArrayList();

	//These hold the line numbers for each statement
	ArrayList generalnum = new ArrayList();
	ArrayList weaponsnum = new ArrayList();
	ArrayList ninjaropenum = new ArrayList();
	ArrayList wormnum = new ArrayList();
	ArrayList sfxnum = new ArrayList();
	ArrayList gfxnum = new ArrayList();

	weaponInfo wepinfo;
	String tempname = "";
	documentIndenter docindent;


	//[General] vars
	String modname = "";
	String description = "";
	String author = "";
	String contact = "";

	//[Nanjarope] vars
	String ropelength = "300";
	String restlength = "27";
	String strength = "4.5";

	//[Worm] vars
	String anglespeed = "159";
	String groundspeed = "8";
	String airspeed = "4";
	String gravity = "100";
	String jumpforce = "-65";
	String airfriction = "0";
	String groundfriction = "0.4";


	//[GFX] vars
	String gcrosshair = "";
	String ghealthbonus = "";
	String ghook = "";
	String gspawn = "";
	String gtagmarker = "";
	String gweaponbonus = "";
	String gchat = "";

	//[SFX] vars
	String splayerbump = "";
	String schat = "";
	String sdeath1 = "";
	String sdeath2 = "";
	String sdeath3 = "";
	String spickupbonus = "";
	String sropethrow = "";

	public mainReader(String path, main par){
		this.path = path;
		this.par = par;
		//Load main.txt
		exec(path+"\\main.txt");
		System.out.println("Compiling: main.txt");

		//Sort the commands into variables
		sortGeneral();
		sortWeapons();
		sortNinjaRope();
		sortWorm();
		sortSFX();
		sortGFX();
		save();
		docindent = new documentIndenter(path+"\\lua\\main.lua");
		savecfg();
		//wepinfo = new weaponInfo("c:\\testmod\\w_missile.txt");
		/*System.out.println("Section: general\n");
		for(int i=0;i<general.size();i++)
			System.out.println((String)general.get(i));
		System.out.println("Section: weapons\n");
		for(int i=0;i<weapons.size();i++)
			System.out.println((String)weapons.get(i));
		System.out.println("Section: ninjarope\n");
		for(int i=0;i<ninjarope.size();i++)
			System.out.println((String)ninjarope.get(i));
		System.out.println("Section: worm\n");
		for(int i=0;i<worm.size();i++)
			System.out.println((String)worm.get(i));*/

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
			//System.out.println("Error: main.txt not found");
			((main)par).addError("Main.txt not found");
			((main)par).exit();
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
		if(com.length() > 1){
			if(current.equalsIgnoreCase("general")){
				general.add(com);
				generalnum.add(""+linenum);
			}else if(current.equalsIgnoreCase("weapons")){
				weapons.add(com);
				weaponsnum.add(""+linenum);
			}else if(current.equalsIgnoreCase("ninjarope")){
				ninjarope.add(com);
				ninjaropenum.add(""+linenum);
			}else if(current.equalsIgnoreCase("worm")){
				worm.add(com);
				wormnum.add(""+linenum);
			}else if(current.equalsIgnoreCase("gfx")){
				gfx.add(com);
				gfxnum.add(""+linenum);
			}else if(current.equalsIgnoreCase("sfx")){
				sfx.add(com);
				sfxnum.add(""+linenum);
			}else if(!getCommand(com).trim().equalsIgnoreCase(""))
				((main)par).addWarning("Statement out of place: \""+getCommand(com)+"\" at line "+linenum+" in main.txt");
		}
	}

	//Method used to change the group that the commands are being added to
	public void getSection(String sect){

		int length = sect.length();
		String msect = sect.substring(1,length-1);
		if(msect.equalsIgnoreCase("general"))
			current = "general";
		else if(msect.equalsIgnoreCase("weapons"))
			current = "weapons";
		else if(msect.equalsIgnoreCase("ninjarope"))
			current = "ninjarope";
		else if(msect.equalsIgnoreCase("worm"))
			current = "worm";
		else if(msect.equalsIgnoreCase("gfx"))
			current = "gfx";
		else if(msect.equalsIgnoreCase("sfx"))
			current = "sfx";
		else
			((main)par).addWarning("Invalid Section: \""+msect+"\" at line "+linenum+" in main.txt");


	}
	public void sortGeneral(){
		for(int i=0;i<general.size();i++){
			String code = (String)general.get(i);
			if(getCommand(code).equalsIgnoreCase("ModName")){
				modname = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("Description")){
				description = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("Author")){
				author = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("Contact")){
				contact = getVar(code);
			}else{
				((main)par).addWarning("Invalid statement: \""+getCommand(code)+"\" at line "+(String)generalnum.get(i)+" in main.txt");
			}

		}
	}

	public void sortNinjaRope(){
		for(int i=0;i<ninjarope.size();i++){
			String code = (String)ninjarope.get(i);
			if(getCommand(code).equalsIgnoreCase("ropelength")){
				ropelength = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("restlength")){
				restlength = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("strength")){
				strength = getVar(code);
			}else{
				((main)par).addWarning("Invalid statement: \""+getCommand(code)+"\" at line "+(String)ninjaropenum.get(i)+" in main.txt");
			}

		}
	}

	public void sortWorm(){
		for(int i=0;i<worm.size();i++){
			String code = (String)worm.get(i);
			if(getCommand(code).equalsIgnoreCase("anglespeed")){
				anglespeed = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("groundspeed")){
				groundspeed = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("airspeed")){
				airspeed = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("gravity")){
				gravity = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("jumpforce")){
				jumpforce = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("airfriction")){
				airfriction = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("groundfriction")){
				groundfriction = getVar(code);
			}else{
				((main)par).addWarning("Invalid statement: \""+getCommand(code)+"\" at line "+(String)wormnum.get(i)+" in main.txt");
			}
		}
	}

	public void sortWeapons(){
		int i2 = 0;
		for(int i=0;i<weapons.size();i++){
			String code = (String)weapons.get(i);
			String code2 = getCommand((String)weapons.get(i));
			if(!code2.equalsIgnoreCase("NumWeapons") && stringStartsWith(code2, "weapon")){
				String code3 = (String)weapons.get(i);

				weapon.add(getVar(code3));
				System.out.println("Compiling: "+(String)weapon.get(i2));

				String fname1 = (String)weapon.get(i2);
				int ilength = fname1.length()-4;
				String fname = fname1.substring(0,ilength);
				wepinfo = new weaponInfo(path+"\\"+(String)weapon.get(i2),path,fname,this, par);
				weaponnames.add(tempname);

				i2++;
			}


		}

	}

	public void sortGFX(){
		int i2 = 0;
		for(int i=0;i<gfx.size();i++){
			String code = (String)gfx.get(i);
			if(getCommand(code).equalsIgnoreCase("crosshair")){
				gcrosshair = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("healthbonus")){
				ghealthbonus = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("hook")){
				ghook = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("spawn")){
				gspawn = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("tagmarker")){
				gtagmarker = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("weaponbonus")){
				gweaponbonus = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("chat")){
				gchat = getVar(code);
			}else{
				((main)par).addWarning("Invalid statement: \""+getCommand(code)+"\" at line "+(String)gfxnum.get(i)+" in main.txt");
			}


		}

	}

	public void sortSFX(){
		int i2 = 0;
		for(int i=0;i<sfx.size();i++){
			String code = (String)sfx.get(i);
			if(getCommand(code).equalsIgnoreCase("playerbump")){
				splayerbump = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("chat")){
				schat = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("death1")){
				sdeath1 = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("death2")){
				sdeath2 = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("death3")){
				sdeath3 = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("pickupbonus")){
				spickupbonus = getVar(code);
			}
			else if(getCommand(code).equalsIgnoreCase("ropethrow")){
				sropethrow = getVar(code);
			}else{
				((main)par).addWarning("Invalid statement: \""+getCommand(code)+"\" at line "+(String)sfxnum.get(i)+" in main.txt");
			}
		}

	}

	public void returnname(String names){
		tempname = names;
	}


	public boolean stringStartsWith(String string, String text){
		int tsize = text.length();
		String string2 = string.substring(0,tsize);
		if(text.equalsIgnoreCase(string2)){
			return true;
		}
		return false;


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
				found = true;
				n1 = i;
				n2 = i+1;
				break;
			}
		}
		if(found){
			String com2 = com.substring(0,n1-1);
			return com2;
		}
		return command;



		}
		//Get's the variable from a script command line
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
	public void saveGeneral(PrintWriter printwriter){
		printwriter.println("-- Created by the LXE Script Basic Compiler");
		printwriter.println("include(\"Common.lua\")");
		printwriter.println("function initialize(player)");
		//Write weapons to script
		for(int i=0;i<weapon.size();i++){
			String wep = (String)weapon.get(i);
			String wname = (String)weaponnames.get(i);
			printwriter.println("weapons.add(\""+wname+"\", \""+wep+"\")");
		}


		//Rope settings
		printwriter.println("rope.length = "+ropelength);
		printwriter.println("rope.restlength = "+restlength);
		printwriter.println("rope.restlength = "+strength);


		//Player settings
		printwriter.println("player.anglespeed = "+anglespeed);
		printwriter.println("player.groundspeed = "+groundspeed);
		printwriter.println("player.airspeed = "+airspeed);
		printwriter.println("player.gravity = "+gravity);
		printwriter.println("player.jumpforce = "+jumpforce);
		printwriter.println("player.airfriction = "+airfriction);
		printwriter.println("player.groundfriction = "+groundfriction);

		//GFX settings

		if(!gcrosshair.trim().equalsIgnoreCase(""))
			printwriter.println("gfx.crosshair = surface(\""+gcrosshair+"\", true)");

		if(!ghealthbonus.trim().equalsIgnoreCase(""))
			printwriter.println("gfx.healthbonus = surface(\""+ghealthbonus+"\", true)");

		if(!ghook.trim().equalsIgnoreCase(""))
			printwriter.println("gfx.hook = surface(\""+ghook+"\", true)");

		if(!gspawn.trim().equalsIgnoreCase(""))
			printwriter.println("gfx.spawn = surface(\""+gspawn+"\", true)");

		if(!gtagmarker.trim().equalsIgnoreCase(""))
			printwriter.println("gfx.tagmarker = surface(\""+gtagmarker+"\", true)");

		if(!gweaponbonus.trim().equalsIgnoreCase(""))
			printwriter.println("gfx.weaponbonus = surface(\""+gweaponbonus+"\", true)");

		if(!gchat.trim().equalsIgnoreCase(""))
			printwriter.println("gfx.chat = surface(\""+gchat+"\", true)");


		//SFX settings

		if(!splayerbump.trim().equalsIgnoreCase(""))
			printwriter.println("sfx.playerbump = sample(\""+splayerbump+"\")");

		if(!schat.trim().equalsIgnoreCase(""))
			printwriter.println("sfx.chat = sample(\""+schat+"\")");

		if(!sdeath1.trim().equalsIgnoreCase(""))
			printwriter.println("sfx.death1 = sample(\""+sdeath1+"\")");

		if(!sdeath2.trim().equalsIgnoreCase(""))
			printwriter.println("sfx.death2 = sample(\""+sdeath2+"\")");

		if(!sdeath3.trim().equalsIgnoreCase(""))
			printwriter.println("sfx.death3 = sample(\""+sdeath3+"\")");

		if(!spickupbonus.trim().equalsIgnoreCase(""))
			printwriter.println("sfx.pickupbonus = sample(\""+spickupbonus+"\")");

		if(!sropethrow.trim().equalsIgnoreCase(""))
			printwriter.println("sfx.ropethrow = sample(\""+sropethrow+"\")");



		printwriter.println("end");


	}

	public void savecfg(){
		File file9;
		file9 = new File(path+"\\lua\\Settings.cfg");

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

		printwriter.println("-- Created by the LXE Script Basic Compiler");
		printwriter.println("[Settings]");
		printwriter.println("Name = "+modname);
		printwriter.println("Description = "+description);
		printwriter.println("Author = "+author);
		printwriter.println("Contact = "+contact);


		printwriter.close();

	}

	public void save(){
		File file9;
		File file1;
		file9 = new File(path+"\\lua\\main.lua");
		file1 = new File(path+"\\lua");
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
		saveGeneral(printwriter);
		//int g = fc.size();
		//for(int i=0;i<g;i++){
			//printwriter.println(encr.encrypt((String)fc.get(i)));
		//}









		printwriter.close();

	}


}