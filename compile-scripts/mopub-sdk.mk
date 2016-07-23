$(JLL_DEPS)/mopub-sdk/: $(JLL_DEPS)/
	cd $(JLL_DEPS)/ && \
	wget https://s3.amazonaws.com/mopub-android-sdk/mopub-android-base-sdk.zip && \
	unzip mopub-android-full-sdk.zip && \
	rm mopub-android-full-sdk.zip

$(JLL_DEPS)/:
	mkdir -p $(JLL_DEPS)/
###
