#include "http.h"
int main()
{
	// while(1)
	{	
		http_get("http://10.0.0.222:8989/route?point=30.46901,104.004875&point=30.469769,104.006259&type=json&locale=zh-CN&key=&elevation=false&profile=hike");
		// http_post_str("http://10.0.0.222:8989/maps/?point=30.467234,104.00537&point=30.467757,104.005412&locale=zh-CN&elevation=false&profile=car&use_miles=false&layer=Esri Aerial");
	}
	return 0;
}