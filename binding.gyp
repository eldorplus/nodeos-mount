{
	"targets": [
		{
			"target_name": "mount",
			"sources": [ "mount.cc" ],
			"include_dirs": [ "<!(node -e \"require('nan')\")" ]
		}
	]
}
