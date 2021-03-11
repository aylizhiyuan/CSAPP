/*
 * @Author: your name
 * @Date: 2021-03-11 12:08:14
 * @LastEditTime: 2021-03-11 12:10:17
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /csapp/code/sum.c
 */

// 一个用来对数组求和的函数
int sum(int *a,int n){
    int i,s = 0;
    for(i=0;i<n;i++){
        s += a[i];
    }
    return s;
}
