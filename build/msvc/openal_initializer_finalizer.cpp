extern "C" {
	void alc_init(void);
	void alc_deinit(void);
}

struct InitializerFinalizerForAL {
	InitializerFinalizerForAL() { alc_init(); }
	~InitializerFinalizerForAL() { alc_deinit(); }
};

InitializerFinalizerForAL ___alInitUninit;

