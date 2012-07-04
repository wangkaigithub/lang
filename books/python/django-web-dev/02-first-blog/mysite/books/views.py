# -*- coding: utf-8 -*-

from django.db.models import Q
from django.shortcuts import render_to_response
from models import Book
from forms import ContactForm
from django.core.mail import send_mail
from django.http import HttpResponseRedirect
from forms import PublisherForm


def search(request):
    # request.GET 返回表单传递的字典数据;
    # request.GET.get(ker, defVal) 返回字典中键为key的值, 若找不到则返回defVal.
    # 注意: 如果使用 request.GET['q'] 访问变量, 在Get数据时 q 不可得,可能引发 KeyError .
    query = request.GET.get('q', '')
    if query:
        qset = (
                # 技术上 Q 对象包含QuerySet.
                # icontains 使用SQL的 LIKE 操作符, 是大小写不敏感的. 
                Q(title__icontains=query) |
                Q(authors__first_name__icontains=query) |
                Q(authors__last_name__icontains=query)
                )
        results = Book.objects.filter(qset).distinct()
    else:
        results = []
    return render_to_response("books/search.html", {
        "results": results,
        "query": query
        })



def contact(request):
    if request.method == 'POST':    # 处理用户提交动作
        # ContactForm会自动校验用户输入.
        form = ContactForm(request.POST)
        if form.is_valid():
            topic = form.data['topic']
            message = form.data['message']
            sender = form.data.get('sender', 'noreply@example.com')
            send_mail('Feedback from your site, topic: %s' % topic,
                    message, sender,
                    ['805899693@qq.com']
                    )
            # 在一个POST请求过后, 如果用户选择刷新页面, 这个请求就重复提交了. 
            # 在POST之后重定向页面是一个有用的模式, 可以避免这样的情况出现.
            return HttpResponseRedirect('/contact/thanks/')
    else:
        form = ContactForm(initial={'sender': 'user@example.com'})
    return render_to_response('books/contact.html', {'form': form})



def thanks(request):
    return render_to_response('books/thanks.html')



def add_publisher(request):
    if request.method == 'POST':
        form = PublisherForm(request.POST)
        if form.is_valid():
            # 从 form_for_model 而来的表单对象包含一个 save() 方法.
            form.save()
            return HttpResponseRedirect('/contact/thanks/')
    else:
        form = PublisherForm()
    return render_to_response('books/add_publisher.html', {'form': form})

