library(ggplot2)

l_100 = read.csv("100000_iters_100_keys", header=FALSE)
l_200 = read.csv("100000_iters_200_keys", header=FALSE)
l_300 = read.csv("100000_iters_300_keys", header=FALSE)
l_400 = read.csv("100000_iters_400_keys", header=FALSE)
l_500 = read.csv("100000_iters_500_keys", header=FALSE)
l_600 = read.csv("100000_iters_600_keys", header=FALSE)
l_700 = read.csv("100000_iters_700_keys", header=FALSE)
l_800 = read.csv("100000_iters_800_keys", header=FALSE)
l_900 = read.csv("100000_iters_900_keys", header=FALSE)
l_1000 = read.csv("100000_iters_1000_keys", header=FALSE)
l_1100 = read.csv("100000_iters_1100_keys", header=FALSE)
l_1200 = read.csv("100000_iters_1200_keys", header=FALSE)
l_1300 = read.csv("100000_iters_1300_keys", header=FALSE)
l_1400 = read.csv("100000_iters_1400_keys", header=FALSE)
l_1500 = read.csv("100000_iters_1500_keys", header=FALSE)
l_1600 = read.csv("100000_iters_1600_keys", header=FALSE)
l_1700 = read.csv("100000_iters_1700_keys", header=FALSE)
l_1800 = read.csv("100000_iters_1800_keys", header=FALSE)
l_1900 = read.csv("100000_iters_1900_keys", header=FALSE)
l_2000 = read.csv("100000_iters_2000_keys", header=FALSE)

df <- data.frame(x = c(l_100$V1, l_200$V1, l_300$V1,  l_400$V1, l_500$V1, l_600$V1, l_700$V1, l_800$V1, l_900$V1, l_1000$V1, l_1100$V1, l_1200$V1, l_1300$V1, l_1400$V1, l_1500$V1, l_1600$V1, l_1700$V1, l_1800$V1, l_1900$V1, l_2000$V1), ggg = factor(rep(1:20, c(1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000))))
df <- df[order(df$x), ]
df$ecdf <- ave(df$x, df$ggg, FUN=function(x) seq_along(x)/length(x))
png('plot.png', width=11, height=8.5, units='in', res=300)
d <- ggplot(df, aes(x, ecdf, colour = ggg), ) + geom_line() + scale_colour_hue(name="Memcached Multi-get latency", labels=c('100','200', '300', '400', '500', '600', '700', '800', '900', '1000', '1100', '1200', '1300', '1400', '1500', '1600', '1700', '1800', '1900', '2000'))
d + coord_cartesian(xlim = c(0, 0.005))
dev.off()
