template<int NumOff, typename In, typename Out, typename F>
void stencil(int n, const In a[], Out r[], In b, F func, const int offsets[])
{
	In neighbourhood[NumOff];
	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < NumOff; ++j)
		{
			int k = i+offsets[j];
			if (0 <= k && k < n) neighbourhood = a[k];
			else neighbourhood[j] = b;
		}
		r[i] = func(neighbourhood);
	}
}


int main ()
{
	stencil()
}
