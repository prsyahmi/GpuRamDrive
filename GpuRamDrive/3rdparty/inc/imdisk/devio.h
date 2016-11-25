typedef safeio_ssize_t (__cdecl *dllread_proc)(void *handle,
					       void *buf,
					       safeio_size_t size,
					       off_t_64 offset);

typedef safeio_ssize_t (__cdecl *dllwrite_proc)(void *handle,
						void *buf,
						safeio_size_t size,
						off_t_64 offset);

typedef int (__cdecl *dllclose_proc)(void *handle);

typedef void * (__cdecl *dllopen_proc)(const char *file,
				       int read_only,
				       dllread_proc *dllread,
				       dllwrite_proc *dllwrite,
				       dllclose_proc *dllclose,
				       off_t_64 *size);

