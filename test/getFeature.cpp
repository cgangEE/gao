#include "head.h"

#define trainData "train.txt"
#define testData "predict.txt"

#define inputData trainData
#define BUF_SIZE 100000

#include "text.h"
#include "blog.h"
#define N 50000

char buf[BUF_SIZE];

vector<Blog> userBlog[N+10];
vector<Blog> testUserBlog[N+10];

Blog statics[N+10];

vector<Blog> train;
vector<Blog> test;

map<string, int> stringToId;
vector<string> keyWords;
int idx = 0;

void outputUserInfo(){
	string fUserName = "user.out";
	FILE *fUser = fopen(fUserName.c_str(), "w");
	if (fUser==NULL){
		fprintf(stderr, "open file<%s> failed\n", fUserName.c_str());
		return;
	}

	fprintf(fUser, "cnt,forward,comment,like,df,dc,dl,maxf,maxc,maxl,minf,minc,minl,text,http,face\n");
	repf(i, 1, idx){
		int cnt = sz(userBlog[i]);
		double forward = statics[i].forward*1./cnt;
		double comment = statics[i].comment*1./cnt;
		double like = statics[i].like*1./cnt;

		double df = 0;
		double dc = 0;
		double dl = 0;

		int maxf = 0;
		int maxc = 0;
		int maxl = 0;

		int minf = INT_MAX;
		int minc = INT_MAX;
		int minl = INT_MAX;

		double len = statics[i].len*1./cnt;
		double http = statics[i].http*1./cnt;
		double face = statics[i].face*1./cnt;

		rep(j, sz(userBlog[i])){
			df += SQR(userBlog[i][j].forward - forward);
			dc += SQR(userBlog[i][j].comment - comment);
			dl += SQR(userBlog[i][j].like - like);
			
			maxf = max(maxf, userBlog[i][j].forward);
			maxc = max(maxc, userBlog[i][j].comment);
			maxl = max(maxl, userBlog[i][j].like);

			minf = min(minf, userBlog[i][j].forward);
			minc = min(minc, userBlog[i][j].comment);
			minl = min(minl, userBlog[i][j].like);
		}

		df /= cnt;
		dc /= cnt;
		dl /= cnt;

		fprintf(fUser, "%d,%.8lf,%.8lf,%.8lf,%.8lf,%.8lf,%.8lf,%d,%d,%d,%d,%d,%d,%.8lf,%.8lf,%.8lf\n",
				cnt,
				forward,
				comment,
				like,
				df,
				dc,
				dl,
				maxf,
				maxc,
				maxl,
				minf,
				minc,
				minl,
				len,
				http,
				face
				);
	}
}

void readTest(){
	FILE *foutput = fopen("test.out", "w");
	if (foutput==NULL){
		fprintf(stderr, "oepn file<%s> failed\n", "test.out");
		return;
	}

	fprintf(foutput, "cnt,uid,userForward,userComment,userLike,text,http,face,forward,comment,like,userId,blogId,date,keyword");

	/*
	rep(i, sz(keyWords)) 
		fprintf(foutput, ",word%d", i);
	*/

	fprintf(foutput, "\n");


	rep(i, sz(test)){
		Blog &blog = test[i];
		int idx = stringToId[blog.userId];

		int cnt = sz(userBlog[idx]);
		int cnt2 = sz(testUserBlog[idx]);

		int keyword = 0;
		rep(j, sz(keyWords))
			keyword += blog.text.getCnt(keyWords[j]);

		Blog &stat = statics[idx];
		fprintf(foutput, "%d,%d,%.8lf,%.8lf,%.8lf,%d,%d,%d,%d,%d,%d,%s,%s,%s,%d",
				cnt+cnt2,
				idx,
				cnt==0?0.:stat.forward*1./cnt,
				cnt==0?0.:stat.comment*1./cnt,
				cnt==0?0.:stat.like*1./cnt,
				blog.len,
				blog.http,
				blog.face,
				blog.forward,
				blog.comment,
				blog.like,
				blog.userId.c_str(),
				blog.blogId.c_str(),
				blog.date.c_str(),
				keyword
				);

		/*
		rep(j, sz(keyWords)){
			fprintf(foutput, ",%d", blog.text.getCnt(keyWords[j]));
		}
		*/
		fprintf(foutput, "\n");

	}

	fclose(foutput);
}


void readTrain(){
	string fTextName = "chinese.out";
	FILE *fText = fopen(fTextName.c_str(), "w");
	if (fText == NULL){
		fprintf(stderr, "open file<%s> failed\n", fTextName.c_str());
		return;
	}

	FILE *file = fopen(inputData, "r");
	if (file==NULL){
		fprintf(stderr, "open file<%s> failed\n", inputData);
		return;
	}

	string fOutputName = "train.out";
	FILE *foutput = fopen(fOutputName.c_str(), "w");
	if (foutput==NULL){
		fprintf(stderr, "open file<%s> failed\n", fOutputName.c_str());
		return;
	}


	while (fgets(buf, BUF_SIZE, file)){
		Blog blog(buf);
		if (rand()%100<=80)
			train.push_back(blog);
		else
			test.push_back(blog);
	}
	fclose(file);


	rep(i, sz(train)){
		Blog &blog =train[i];
		int userIdx = stringToId[blog.userId];
		if (userIdx == 0){
			userIdx = ++idx;
			stringToId[blog.userId] = userIdx;
		}
		userBlog[userIdx].push_back(blog);
		fprintf(fText, "%s", blog.text.s.c_str());
	}

	// get statics for each users

	repf(i, 1, idx){
		rep(j, sz(userBlog[i]))
			statics[i] = statics[i] + userBlog[i][j];
	}


	outputUserInfo();
	//put test data into userBlog

	rep(i, sz(test)){
		Blog &blog = test[i];
		int userIdx = stringToId[blog.userId];
		if (userIdx == 0){
			userIdx = ++idx;
			stringToId[blog.userId] = userIdx;
		}
		testUserBlog[userIdx].push_back(blog);
		fprintf(fText, "%s", blog.text.s.c_str());
	}

	// output features of each train data


	fprintf(foutput, "cnt,uid,userForward,userComment,userLike,forward,comment,like,text,http,face,date,keyword");
	fprintf(foutput, "\n");

	rep(i, sz(train)){
		Blog &blog = train[i];
		int userIdx = stringToId[blog.userId];

		Blog &stat = statics[userIdx];
		int cnt = userBlog[userIdx].size();
		int cnt2 = testUserBlog[userIdx].size();

		int keyword = 0;
		rep(j, sz(keyWords))
			keyword += blog.text.getCnt(keyWords[j]);

		fprintf(foutput, "%d,%d,%.8lf,%.8lf,%.8lf,%d,%d,%d,%d,%d,%d,%s,%d", 
				cnt+cnt2,
				userIdx,
				stat.forward*1./cnt,
				stat.comment*1./cnt,
				stat.like*1./cnt,
				blog.forward,
				blog.comment,
				blog.like,
				blog.len,
				blog.http,
				blog.face,
				blog.date.c_str(),
				keyword
				);
		fprintf(foutput, "\n");
	}
	fclose(foutput);

}

void readKeyWord(){
	string fileName = "KeyWords/keyWord.out";
	FILE *file = fopen(fileName.c_str(), "r");

	char tmp[100];

	while (fgets(buf, BUF_SIZE, file)){
		sscanf(buf, "%s", tmp);
		keyWords.push_back(tmp);
	}
}

int main(){
	srand(time(NULL));
	readKeyWord();
	readTrain();	
	readTest();
	return 0;
}

