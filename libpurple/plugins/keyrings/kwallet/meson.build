if kwallet.found()
	kwallet_moc = qt5.preprocess(
		moc_headers: 'purplekwallet.h',
		dependencies: qt5_dep,
	)

	kwallet_sources = [
		'purplekwallet.cpp',
		'purplekwallet.h',
		kwallet_moc,
	]

	kwallet_plugin = library('purplekwallet', kwallet_sources,
	    dependencies : [kwallet, qt5_dep, libpurple_dep],
	    name_prefix : '',
	    install : true, install_dir : PURPLE_PLUGINDIR)
endif

