#pragma once
#ifndef LinkList_HH
#define LinkList_HH
#include <vector>

template <typename type>
struct Node
{
	type value;
	Node *next;
	bool remove;
	Node()
	{
		next = nullptr;
		remove = false;
	}
};


template <typename type>
class LinkList
{

public:
	typedef Node<type>* iterator;
	iterator begin();
	iterator end();
	iterator operator ++();
	iterator operator ++(int);
	iterator next();
	Node<type>* _cur_it;
	//static type* 

public:
	int Length;
	Node<type> *head;
	//the "next" of tail is the previous
	Node<type> *tail;

	LinkList();
	bool add_front(type);//
	void add_back(type);
	bool pre_remove(type);
	bool remove(int);//don't use
	bool remove(type);
	type getValue(int);//don't use
	int getIndex(type);//don't use
	int getLength();//
	Node<type>* getHead();//
	bool addnode(int ,type);//don't use
	type front();
	type back();
	void pop_front();
	void pre_pop_front();
	void pop_back();
	int size();
	void push_front(type );
	void push_back(type );
	void clear();
	void sort_pointer(int t);

	void recycle();
	~LinkList();//

private:

	//static const int MAX_REMOVE_LEN = 800;
	int _n;
	//type _remove_list[MAX_REMOVE_LEN];
	std::vector<type> _remove_list;
	bool _pop_front;

};

template <typename type>
typename LinkList<type>::iterator LinkList<type>::begin()
{
	//_cur_it = head->next;
	return head->next;
}

template <typename type>
typename LinkList<type>::iterator LinkList<type>::end()
{
	return tail->next;
}

template <typename type>
typename LinkList<type>::iterator LinkList<type>::operator++()
{
	if(!_cur_it)
	{
		_cur_it = head->next;
	}
	if(_cur_it)
		_cur_it = _cur_it->next;
	if(_cur_it==tail->next)
	{
		Node<type>* temp = _cur_it;
		_cur_it = head->next;
		return temp;
	}
	return _cur_it;
}

template <typename type>
typename LinkList<type>::iterator LinkList<type>::next()
{
	if(!_cur_it)
	{
		_cur_it = head->next;
	}
	if(_cur_it)
		_cur_it = _cur_it->next;
	if(_cur_it==tail->next)
	{
		Node<type>* temp = _cur_it;
		_cur_it = head->next;
		return temp;
	}
	return _cur_it;
}

template <typename type>
typename LinkList<type>::iterator LinkList<type>::operator ++(int)
{
	return nullptr;
}

template <typename type>
type LinkList<type>::front()
{
	return head->next->value;
}

template <typename type>
type LinkList<type>::back()
{
	return tail->next->value;
}

template <typename type>
void LinkList<type>::pre_pop_front()
{
	_pop_front = true;
}

template <typename type>
void LinkList<type>::recycle()
{
	for(int i=_remove_list.size()-1;i>=0;--i)
	{
		remove(_remove_list[i]);
	}
	if(_pop_front)
	{
		pop_front();
		_pop_front = false;

	}
	//��������ʧЧ
	_cur_it = head->next;
	_n = 0;
	_remove_list.clear();
}

template <typename type>
void LinkList<type>::pop_front()
{
	Node<type> *temp = head->next;
	if(!temp)
	{
		return;
	}
	head->next = temp->next;
	if(tail->next==temp)
	{
		tail->next = head;

	}
	delete temp;
	temp = 0;
	--Length;
}

template <typename type>
void LinkList<type>::pop_back()
{
	Node<type> *temp = tail->next;
	if(temp==head)
		return;
	Node<type> *p = head;
	while(p->next!=temp)
	{
		p = p->next;
	}
	tail->next = p;
	delete temp;
	--Length;
}

template <typename type>
int LinkList<type>::size()
{
	return Length;
}

template <typename type>
void LinkList<type>::push_front(type s)
{
	add_front(s);
}

template <typename type>
void LinkList<type>::push_back(type s)
{
	add_back(s);
}

template <typename type>
void LinkList<type>::clear()
{
	Node<type> *temp = head->next;
	Node<type> *temp1 = nullptr;
	int count = 0;
	while(count < Length)
	{
		temp1 = temp;
		if(temp1 != nullptr)
		{
			temp = temp->next;
			delete temp1;
			temp1 = nullptr;
		}
		count++;
	}
}

template <typename type>
void LinkList<type>::sort_pointer(int t)
{
	if(!head->next)
		return;
	switch (t)
	{
		//>
	case 1:
		{
			Node<type> *p1,*p2,*max;
			type a;
			p1=p2=head->next;
			while(p1)
			{
				p2=p1->next;
				max = p1;

				while (p2)
				{
					if(*(p2->value)>*(max->value))
					{
						max = p2;

					}

					p2=p2->next;
				}

				a = max->value;
				max->value = p1->value;
				p1->value = a;

				p1=p1->next;

			}
		}
		break;
		//<
	default:
		{

		}
		break;
	}
}

template <typename type>
LinkList<type>::LinkList()
{

	Length = 0;

	//empty head use for convenience
	head = (new Node<type>());
	head->value = nullptr;
	head->next = nullptr;

	//the tail is just for convenience
	tail = new Node<type>();
	tail->next = nullptr;
	tail->next = head;

	_cur_it = head->next;

	_pop_front = false;
	_n = 0;

}


template <typename type>
bool LinkList<type>::add_front(type value)
{
	//add front
	Node<type> *node = new Node<type>();
	node->value = value;
	node->next = head->next;
	head->next = node;
	if(node->next == nullptr)
	{
		tail->next = node;
	}
	++Length;
	return true;
}

template <typename type>
void LinkList<type>::add_back(type value)
{
	//add back
	Node<type> *node = new Node<type>();
	node->value = value;
	node->next = nullptr;
	tail->next->next = node;
	tail->next = node;
	++Length;
}

template <typename type>
bool LinkList<type>::remove(int index)
{



}

template <typename type>
bool LinkList<type>::pre_remove(type e)
{
	_remove_list.push_back(e);
	//_remove_list[_n++] = e;
	return true;

}

template <typename type>
bool LinkList<type>::remove(type e)
{
	Node<type> *pre = head;
	if(!head)
		return false;
	Node<type> *p = head->next;
	if(!p)
		return false;
	while(p->value!=e)
	{
		pre = p;
		p = p->next;
	}
	if(p==tail->next)
	{
		tail->next = pre;
	}
	pre->next = p->next;
	delete p;

	--Length;
	return true;

}


template <typename type>


bool LinkList<type>::addnode(int index, type value)
{

}
template <typename type>
int LinkList<type>::getLength()
{
	return Length;
}
template <typename type>


Node<type>* LinkList<type>::getHead()
{
	return head;
}
template <typename type>
type LinkList<type>::getValue(int index)
{

}
template <typename type>
int LinkList<type>::getIndex(type value)
{

}
template <typename type>
LinkList<type>::~LinkList()
{
	clear();
	delete head;
	delete tail;
}
#endif