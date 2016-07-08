# KanoOS_piTop
Scripts for Kano OS on the pi-top system to properly shutdown hub and control screen brightness

#Install Instructions
cd
cd Downloads                                            //Please be aware this folder is not created until you download something (via Chrome or similar)
git clone git://github.com/Tandokude/KanoOS_piTop       //Download latest folder from git
cd KanoOS_piTop                                         //Go into folder
chmod +x install*                                       //Make install scripts executable

#install brightness
sudo ./install-brightness

#install shutdown                                       //Installs script to shutdown hub at kano OS shutdown request
sudo ./install-shutdown

#Control brightness
brightness increase
brightness decrease
brightness new_value                                    //Allows an exact setting between 3-10
