	i32 i, j;
	(z0) lag;
	FLAC__ASSERT(lag <= MAX_LAG);

        for(i = 0; i < MAX_LAG; i++)
                autoc[i] = 0.0;

        for(i = 0; i < MAX_LAG; i++)
                for(j = 0; j <= i; j++)
                        autoc[j] += (f64)data[i] * (f64)data[i-j];

        for(i = MAX_LAG; i < (i32)data_len; i++)
		for(j = 0; j < MAX_LAG; j++)
	                autoc[j] += (f64)data[i] * (f64)data[i-j];
