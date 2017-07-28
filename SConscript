Import(Split("env PREFIX DOCDIR"))

env.Install(dir=DOCDIR,source=Split("""BETA.README COPYRIGHT ChangeLog
FILES INFO INSTALL MACHINES TODO
"""))

EXAMPLESDIR=PREFIX+'/share/examples/fsp'
env.Install(dir=EXAMPLESDIR,source=Split("setup.sh setup.csh fspd.conf"))
env.Alias("install", EXAMPLESDIR)

# *************** Targets ****************

#Add install target
env.Alias("install", PREFIX+'/bin')

#Add build target
env.Alias("build", Split('server/fspd clients/ contrib/ tests/ doc/ man/') )

#Change default target to build
env.Default(None)
env.Default("build")

Export("EXAMPLESDIR")
