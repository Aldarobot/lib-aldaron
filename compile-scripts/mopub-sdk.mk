mopub-sdk: $(shell echo $(JLL_HOME))/deps
	cd $(shell echo $(JLL_HOME))/deps && \
	wget https://s3.amazonaws.com/mopub-android-sdk/mopub-android-full-sdk.zip && \
	unzip mopub-android-full-sdk.zip && \
	rm mopub-android-full-sdk.zip

$(JLL_DEPS):
	mkdir -p $(shell echo $(JLL_HOME))/deps
###
