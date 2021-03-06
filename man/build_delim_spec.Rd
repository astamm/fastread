% Generated by roxygen2 (4.0.1): do not edit by hand
\name{build_delim_spec}
\alias{build_delim_spec}
\title{Build a spec from a csv file.}
\usage{
build_delim_spec(file, delim = NULL, parsers = NULL, col_names = TRUE,
  quote = "\\"", na_strings = "NA", n = 0, skip = 0, comment_char = "", double_escape = FALSE, backslash_escape = FALSE)
}
\arguments{
\item{delim}{Delimiter between lines.}

\item{n}{If not 0, parse at most this many lines from the file.}

\item{skip}{Skip this many lines before beginning parsing.}

\item{quote}{Quoting character. Quotes are not preserved in the output.}

\item{double_escape}{Does doubling the quoting character escape it?}

\item{backslash_escape}{Do backslashes escape the following character?}

\item{parsers}{A list of parsers used to control how each column is turned
  into an R data frame. The elements of \code{parsers} can be named to match
  the columns. Any columns not included will not be included in the output
  data frame.

  Alternatively, you can use string where each character represents
  one column. c = character, d = double, i = integer, l = logical and
  \code{_} skips the column.}

\item{col_names}{A character vector naming the columns.}

\item{na_strings}{A character vector of providing strings which should be
converted to NA in R.}
}
\description{
Builds a complete specification to parse a csv file, including guessing
the column parsers if they're not suppled.
}
\keyword{internal}

