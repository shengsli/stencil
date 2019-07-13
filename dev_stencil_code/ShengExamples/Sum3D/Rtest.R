args = commandArgs(trailingOnly=TRUE)
if (length(args)!=2) {
  stop("Usage: Rscript Rtest.R mydata.csv mygraph.pdf", call.=FALSE)
} 

library(plyr)
library (ggplot2)

MyData <- read.csv(args[1])

MySummary2 <- ddply(MyData, c("nthreads"), summarise,
               N       = length(time),
               runtime = mean(time),
               sd      = sd(time),
               se      = sd / sqrt(N)
)

ggplot(MySummary2, aes(x=nthreads, y=runtime)) + 
    geom_bar(position=position_dodge(), stat="identity", fill = "#FF6666") +
    geom_errorbar(aes(ymin=runtime-se, ymax=runtime+se),
                  width=.2,                    # Width of the error bars
                  position=position_dodge(.9))

ggsave(args[2])