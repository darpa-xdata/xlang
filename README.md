This vagrant project is for the xlang-vm project under XDATA. Follow these instructions.


Install and configure Vagrant<br>
<code>sudo apt-get install vagrant</code>

(Optional) Add Vagrant plugin that keeps Virtual Box Guest Additions in sync.<br>
<code>vagrant plugin install vagrant-vbguest</code>

Clone this repo<br>

Go to the repo directory and start the environment<br>
<code>vagrant up</code><br>

After it boots, log in to the VM and switch to the hdfs user<br>
<code>vagrant ssh</code><br>


To add your software to the VM:<br>
1. Create a sensibly named deployment script in the /scripts directory. This script should download, intall, and configure all of your software from a bare operating system. This is where the majority of your development will be.<br>
2. Store any custom files or code in the files directory or call them from their source location with wget. Examples are in the examples.sh.<br>
3. Add a line to deploy.sh that calls your deployment script. # be sure to comment what it does here.<br>

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
