#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

void readcb(struct bufferevent* bev,void* ){
	
	char buf[1024];
	int r = bufferevent_read(bev,buf,1024);
	if( r > 0)
		bufferevent_write(bev,buf,r);
}

void writecb(struct bufferevent* bev,void* ){

	auto out = bufferevent_get_output(bev);
	int len = evbuffer_get_length(out);
	printf(" write buffer len:%d\n",len);


}
void eventcb(struct bufferevent* bev , short what,void* ){

	if( what & BEV_EVENT_EOF){
		printf("EOF\n");
		bufferevent_free(bev);
	}
}
void on_accept(int fd,short,void* arg){
	struct event_base* base = reinterpret_cast<struct event_base*>(arg);
	struct sockaddr_in peer_addr;
	memset(&peer_addr,0,sizeof(peer_addr));
	socklen_t len=0;
	int clientfd = accept(fd,(struct sockaddr*)&peer_addr,&len);
	evutil_make_socket_nonblocking(clientfd);
	auto bv = bufferevent_socket_new(base,clientfd,BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(bv,readcb,writecb,eventcb,base);
	bufferevent_enable(bv,EV_READ );
	
}

int main(){
	int fd = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(5000);
	addr.sin_addr.s_addr = INADDR_ANY;
	int ret = bind(fd,(struct sockaddr*)&addr,sizeof(addr));
	assert(ret!=-1);
	ret = listen(fd,5);
	assert(ret!=-1);
	struct event_base* base = event_base_new();
	struct event* ev = event_new(base,fd,EV_READ|EV_PERSIST,on_accept,base);
	//struct event* ev = event_new(base,fd,EV_READ,on_accept,base);
	event_add(ev,NULL);
	event_base_dispatch(base);
	event_free(ev);
	event_base_free(base);
	libevent_global_shutdown();
	return 0;
}
