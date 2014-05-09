This vagrant project is for the xlang-vm project under XDATA. Follow these instructions.


Install and configure Vagrant<br>
<code>sudo apt-get install vagrant</code>

(Optional) Add Vagrant plugin that keeps Virtual Box Guest Additions in sync.<br>
<code>vagrant plugin install vagrant-vbguest</code>

Clone this repo<br>
<code>git clone https://github.com/darpa-xdata/xlang-vm.git</code>

Go to the repo directory and start the environment (vagrant commands must be typed in the repo root dir where the Vagrant file resides)<br>
<code>vagrant up</code><br>

After it boots, log in to the VM<br>
<code>vagrant ssh</code><br>


To add your software to the VM:<br>
1. Create a sensibly named deployment script in the /scripts directory. This script should download, intall, and configure all of your software from a bare operating system. This is where the majority of your development will be.<br>
2. Store any custom files or code in the files directory or call them from their source location with wget. Examples are in the examples.sh.<br>
3. Add a line to deploy.sh that calls your deployment script. # be sure to comment what it does here.<br>
4. Commit your changes back to this github repository early and often so everybody can test and benfit from them.<br>

Other useful vagrant commands:
vagrant suspend
Saves the current running state of the machine and stops it. vagrant up will resume.

vagrant halt
Gracefully shuts down the vm. vagrant up will boot it back up.

vagrant destroy
Destroys the vm (and all the cruft inside of it). vagrant up will reprovision and run the deploy scripts again.

More info at http://www.vagrantup.com/

Other notes:

To have Vagrant sync the Guest Additions for Virtual Box do the following:
$ # For vagrant < 1.1.5:
$ # vagrant gem install vagrant-vbguest

$ # For vagrant 1.1.5+ (thanks Lars Haugseth):
$ vagrant plugin install vagrant-vbguest

Troubleshooting:
If deployment fails because the /vagrant directory on the guest operating system fails to mount, this is potentially an incompatibility with VirtualBox Guest Additions. To fix this, <code>vagrant ssh</code> into the system and run the following command:<br>
<code>sudo ln -s /opt/VBoxGuestAdditions-4.3.10/lib/VBoxGuestAdditions /usr/lib/VBoxGuestAdditions</code><br>
Then exit the guest OS and type <code>vagrant reload</code>. 

