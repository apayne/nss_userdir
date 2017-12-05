# Introduction 

This is a private copy of the nss_userdir module, an implementation of 
Dan Rench's userdir concept for Name Service Switch.  Using this module, 
you should be able to implement a userdir layout alongside the 
traditional /etc/passwd and /etc/group files.

Essentially, it does away with the concept of a single passwd file, and 
instead uses the native filesystem to record and store user information 
in separate, distinct files.


# Attribution

The original userdir webpage is more or less lost to antiquity, but the 
original format (and concepts) can be found at the following web 
address:

* http://web.archive.org/web/20120223133916/http://dren.ch/userdir/

As even archive.org can sometimes loose pages due to hardware failures, I have taken the 
liberty of including a webscrape of the original author's page for userdir, included in a 
separate directory.  The webscrape outlines the concepts, provides a FAQ, and even includes 
a bit of supporting code.  If any content in the webscrape represents a problem with 
licensing or copyright, I will immediately remove the files; the link through archive.org 
will still be available.

I am NOT the original author of this patch, the supporting code, or any of the documentation 
in the webscrape, although I have made an effort to make the initial commit in the author's 
"name", so that they are respected for their work.  Unless a commit in the repository for 
the code has my name next to it, I won't be able to answer questions.

There was an original README that came with this source, but I needed to include additional 
notations without distrubing it, so this file is named README.md, and the original stays the 
same.  There you will find instructions on how to use it.

